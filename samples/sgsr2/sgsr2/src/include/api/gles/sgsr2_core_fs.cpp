//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include <unordered_map>
#include <GLES3/gl32.h>
#include "api/gles/sgsr2_core_fs.h"
#include <android/log.h>

#define SGSR2_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "QCSGSR2", __VA_ARGS__))
#define SGSR2_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "QCSGSR2", __VA_ARGS__))
#define SGSR2_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "QCSGSR2", __VA_ARGS__))

using namespace SGSR2_Frag;

static bool checkGlError(const char *funcName) {
    GLint err = glGetError();
    if (err != GL_NO_ERROR) {
        SGSR2_LOGE("GL error after %s(): 0x%08x\n", funcName, err);
        return true;
    }
    return false;
}

static const std::unordered_map<std::string, uint32_t> resourceNameIdentifierMap =
    {
        {"InputOpaqueColor",           SGSR2_RESOURCE_IDENTIFIER_INPUT_OPAQUE_COLOR},
        {"InputColor",                 SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR},
        {"InputDepth",                 SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH},
        {"InputVelocity",              SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY},
        {"MotionDepthClipAlphaBuffer", SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER},
        {"PrevOutput",                 SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT},
        {"Output",                     SGSR2_RESOURCE_IDENTIFIER_OUTPUT},
    };

void SGSR2_Core::DestroyResources() {
    for (int32_t i = 0; i < RESOURCE_COUNT; i++) {
        Resource &resource = resources[i];
        if (resource.resource) {
            GLuint id = resource.resource;
            glDeleteTextures(1, &id);
        }
    }
    glutil::Buffer::destroy(m_Sgsr2Ubo);
    glDeleteFramebuffers(1, &m_ConvertFBHandle);
    glDeleteFramebuffers(1, &m_UpscaleFBHandle);
}

void SGSR2_Core::CreateShaderBlobFromPass(SgsrPass pass, ShaderBlob *shaderBlob) {
    switch (pass) {
        case SGSR2_PASS_CONVERT2FRAG: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_convert2.fs"
    ;
            break;
        }
        case SGSR2_PASS_UPSCALE2FRAG: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_upscale2.fs"
    ;
            break;
        }

        default:
            break;

    }

}

void SGSR2_Core::RegisterResource(const Resource *inResource, uint32_t internalId) {
    //Put in resource in resource lists
    Resource *resource = &resources[internalId];
    resource->resourceDescription = inResource->resourceDescription;
    resource->resource = inResource->resource;
    //SGSR2_LOGI("Register Texture Name %s Texture ID: %d",resource->resourceDescription.name.c_str(), static_cast<GLuint>(reinterpret_cast<uintptr_t>(resource->resource)));
}

void SGSR2_Core::CreatePipeline(SgsrPass pass, Pipeline *outPipeline) {
    ShaderBlob blob = {};
    CreateShaderBlobFromPass(pass, &blob);

    memset(outPipeline, 0, sizeof(Pipeline));

    std::string vertexShaderStr =

        #include "glsl/sgsr2_2pass_vertex.vs"
            ;

    outPipeline->program = glutil::createGraphicsProg(vertexShaderStr, blob.cShaderStr);
    assert(outPipeline->program);
}

void SGSR2_Core::DestroyPipelines() {
    if (pipelineConvert2Frag.program) {
        glDeleteProgram(pipelineConvert2Frag.program);
    }

    if (pipelineUpscale2Frag.program) {
        glDeleteProgram(pipelineUpscale2Frag.program);
    }

}

void SGSR2_Core::CreateResource(const CreateResourceDescription *createResourceDescription) {

    Resource *resource = &resources[createResourceDescription->internalId];
    resource->resourceDescription = createResourceDescription->resourceDescription;

    GLuint texture;
    glGenTextures(1, &texture);
    resource->resource = texture;
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    GLenum format = static_cast<GLenum>(reinterpret_cast<uintptr_t>(createResourceDescription->resourceDescription.format));
    glTexStorage2D(GL_TEXTURE_2D, 1, format,
                   createResourceDescription->resourceDescription.width,
                   createResourceDescription->resourceDescription.height);

    if (createResourceDescription->initData) {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        createResourceDescription->resourceDescription.width,
                        createResourceDescription->resourceDescription.height,
                        glutil::getFormatFromSurfaceFormat(format),
                        glutil::getTypeFromSurfaceFormat(format),
                        createResourceDescription->initData);
    }
    //SGSR2_LOGI("CreateResource Texture Name:%s Texture ID: %d, internal id: %d format: 0x%08x",createResourceDescription->resourceDescription.name.c_str(), texture, createResourceDescription->internalId, format);
    checkGlError("CreateResource error");

}

void SGSR2_Core::Init(uint32_t inputWidth, uint32_t inputHeight, uint32_t outputWidth,
                      uint32_t outputHeight) {
    m_FrameIdx = 0;

    std::vector<ResourceDescription> internalSurfaceDesc;

    internalSurfaceDesc.push_back(
        {"PrevOutput", reinterpret_cast<void *>(GL_RGBA8), outputWidth, outputHeight});
    internalSurfaceDesc.push_back(
        {"Output", reinterpret_cast<void *>(GL_RGBA8), outputWidth, outputHeight});

    internalSurfaceDesc.push_back(
        {"MotionDepthClipAlphaBuffer", reinterpret_cast<void *>(GL_RGBA16F), inputWidth,
         inputHeight});

    uint32_t internalId = 0;
    for (int32_t i = 0; i < internalSurfaceDesc.size(); i++) {
        CreateResourceDescription createResourceDescription;

        std::unordered_map<std::string, uint32_t>::const_iterator got = resourceNameIdentifierMap.find(
            internalSurfaceDesc[i].name);
        if (got != resourceNameIdentifierMap.end()) {
            internalId = got->second;
        }

        createResourceDescription = {internalSurfaceDesc[i], internalSurfaceDesc[i].name,
                                     internalId};
        CreateResource(&createResourceDescription);
    }

    //Create samplers
    glGenSamplers(1, &m_NearestClamp);
    glSamplerParameteri(m_NearestClamp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_NearestClamp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_NearestClamp, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_NearestClamp, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glSamplerParameteri(m_NearestClamp, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenSamplers(1, &m_LinearClamp);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_Sgsr2Ubo = glutil::Buffer::create(GL_UNIFORM_BUFFER, sizeof(Sgsr2Ubo), GL_DYNAMIC_DRAW);

    //Create FBO for 2-pass fragment
    {
        glGenFramebuffers(1, &m_ConvertFBHandle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_ConvertFBHandle);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER].resource,
                               0);

        GLenum glAttachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, glAttachments);

        const GLenum checkFramebufferStatusResult = glCheckFramebufferStatus(
            GL_DRAW_FRAMEBUFFER);
        assert(checkFramebufferStatusResult == GL_FRAMEBUFFER_COMPLETE);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    {
        glGenFramebuffers(1, &m_UpscaleFBHandle);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_UpscaleFBHandle);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               resources[SGSR2_RESOURCE_IDENTIFIER_OUTPUT].resource,
                               0);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                               resources[SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT].resource, 0);

        const GLenum checkFramebufferStatusResult = glCheckFramebufferStatus(
            GL_DRAW_FRAMEBUFFER);
        assert(checkFramebufferStatusResult == GL_FRAMEBUFFER_COMPLETE);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }
    CreatePipeline(SGSR2_PASS_CONVERT2FRAG, &pipelineConvert2Frag);
    CreatePipeline(SGSR2_PASS_UPSCALE2FRAG, &pipelineUpscale2Frag);
    initVertexAttribute();
}

void SGSR2_Core::initVertexAttribute() {
    // Initialize VBO, EBO and VAO
    static const GLfloat vertices[5 * 4] = {
        // pos                          //texture coordinate
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f, // top right
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f, // bot right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bot left
    };

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                          (void *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

}

void SGSR2_Core::ExecuteJobGraphics(SgsrPass pass, Sgsr2Ubo &ubo) {
    int pingpong = m_FrameIdx % 2;
    switch (pass) {
        case SGSR2_PASS_CONVERT2FRAG: {

            glBindFramebuffer(GL_FRAMEBUFFER, m_ConvertFBHandle);
            glViewport(0, 0, ubo.renderSize.x, ubo.renderSize.y);

            glUseProgram(pipelineConvert2Frag.program);
            int index = 0;
            index = 1;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH].resource);
            glBindSampler(index, m_NearestClamp);

            index = 2;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY].resource);

            m_Sgsr2Ubo.bindBase(0);
            break;
        }
        case SGSR2_PASS_UPSCALE2FRAG: {
            glBindFramebuffer(GL_FRAMEBUFFER, m_UpscaleFBHandle);
            glViewport(0, 0, ubo.outputSize.x, ubo.outputSize.y);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   resources[SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT +
                                             (1 - pingpong)].resource,
                                   0);
            GLenum glAttachments[1] = {GL_COLOR_ATTACHMENT0};
            glDrawBuffers(1, glAttachments);
            glUseProgram(pipelineUpscale2Frag.program);

            int index = 1;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT + pingpong].resource);
            glBindSampler(index, m_LinearClamp);

            index = 2;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER].resource);
            glBindSampler(index, m_LinearClamp);

            index = 3;
            glActiveTexture(GL_TEXTURE0 + index);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR].resource);

            m_Sgsr2Ubo.bindBase(0);
            break;
        }
        default:
            break;
    }
    glBindVertexArray(m_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


}

void SGSR2_Core::ExecuteSGSR2(SGSR2ResourceDescription *description) {
    Sgsr2Ubo u{};
    u.renderSize.x = description->renderSize[0];
    u.renderSize.y = description->renderSize[1];
    u.outputSize.x = description->outputSize[0];
    u.outputSize.y = description->outputSize[1];
    u.renderSizeRcp.x = description->renderSizeRcp[0];
    u.renderSizeRcp.y = description->renderSizeRcp[1];
    u.outputSizeRcp.x = description->outputSizeRcp[0];
    u.outputSizeRcp.y = description->outputSizeRcp[1];
    u.jitterOffset.x = description->jitterOffset[0];
    u.jitterOffset.y = description->jitterOffset[1];
    u.scaleRatio.x = description->scaleRatio[0];
    u.scaleRatio.y = description->scaleRatio[1];
    memcpy(u.clipToPrevClip, description->clipToPrevClip, 16 * sizeof(float));
    u.cameraFovAngleHor = description->cameraFovAngleHor;
    u.minLerpContribution = description->minLerpContribution;
    u.reset = description->reset;
    u.bSameCamera = description->sameCameraFrmNum;

    RegisterResource(&description->colorTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR);
    RegisterResource(&description->depthTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH);
    RegisterResource(&description->velocityTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY);

    m_Sgsr2Ubo.subData(&u);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(false);

    ExecuteJobGraphics(SGSR2_PASS_CONVERT2FRAG, u);
    ExecuteJobGraphics(SGSR2_PASS_UPSCALE2FRAG, u);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(true);

    int pingpong = m_FrameIdx % 2;
    description->outputTexture = resources[SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT +
                                           (1 - pingpong)];

    m_FrameIdx++;
}

void SGSR2_Core::Destroy() {
    DestroyPipelines();
    DestroyResources();
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
}
