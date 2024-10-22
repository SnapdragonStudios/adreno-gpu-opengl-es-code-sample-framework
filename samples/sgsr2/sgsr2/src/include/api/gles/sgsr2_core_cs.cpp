//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include <unordered_map>
#include "api/gles/sgsr2_core_cs.h"
#include <android/log.h>
#include <GLES3/gl32.h>

#define SGSR2_LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "QCSGSR2", __VA_ARGS__))
#define SGSR2_LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "QCSGSR2", __VA_ARGS__))
#define SGSR2_LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "QCSGSR2", __VA_ARGS__))

using namespace SGSR2_Comp;

inline uint32_t divideRoundUp(uint32_t dividend, uint32_t divisor) {
    return (dividend + divisor - 1) / divisor;
}

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
        {"MotionDepthAlphaBuffer",     SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_ALPHA_BUFFER},
        {"YCoCgColor",                 SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR},
        {"MotionDepthClipAlphaBuffer", SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER},
        {"PrevLumaHistory",            SGSR2_RESOURCE_IDENTIFIER_PREV_LUMA_HISTORY},
        {"LumaHistory",                SGSR2_RESOURCE_IDENTIFIER_LUMA_HISTORY},
        {"PrevHistoryOutput",          SGSR2_RESOURCE_IDENTIFIER_PREV_HISTORY_OUTPUT},
        {"HistoryOutput",              SGSR2_RESOURCE_IDENTIFIER_HISTORY_OUTPUT},
        {"SceneColorOutput",           SGSR2_RESOURCE_IDENTIFIER_UPSCALED_OUTPUT}
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
}

void SGSR2_Core::CreateShaderBlobFromPass(SgsrPass pass, ShaderBlob *shaderBlob) {
    switch (pass) {
        case SGSR2_PASS_CONVERT: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_convert.comp"
    ;
            break;
        }

        case SGSR2_PASS_ACTIVATE: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_activate.comp"
    ;
            break;
        }

        case SGSR2_PASS_UPSCALE: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_upscale.comp"
    ;
            break;
        }

        case SGSR2_PASS_CONVERT2PASS: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_convert2pass.comp"
    ;
            break;
        }

        case SGSR2_PASS_UPSCALE2PASS: {
            shaderBlob->cShaderStr =
#include "glsl/sgsr2_upscale2pass.comp"
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
    outPipeline->program = glutil::createCompProg(blob.cShaderStr, blob.shaderHandle);

}

void SGSR2_Core::DestroyPipelines() {
    if (pipelineConvert.program) {
        glDeleteProgram(pipelineConvert.program);
    }

    if (pipelineActivate.program) {
        glDeleteProgram(pipelineActivate.program);
    }

    if (pipelineUpscale.program) {
        glDeleteProgram(pipelineUpscale.program);
    }

    if (pipelineConvert2Pass.program) {
        glDeleteProgram(pipelineConvert2Pass.program);
    }

    if (pipelineUpscale2Pass.program) {
        glDeleteProgram(pipelineUpscale2Pass.program);
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
                      uint32_t outputHeight, uint8_t algoMode) {
    m_FrameIdx = 0;
    m_Mode = algoMode;

    std::vector<ResourceDescription> internalSurfaceDesc;
    internalSurfaceDesc.push_back(
        {"SceneColorOutput", reinterpret_cast<void *>(GL_RGBA16F), outputWidth, outputHeight});
    internalSurfaceDesc.push_back(
        {"PrevHistoryOutput", reinterpret_cast<void *>(GL_RGBA16F), outputWidth, outputHeight});
    internalSurfaceDesc.push_back(
        {"HistoryOutput", reinterpret_cast<void *>(GL_RGBA16F), outputWidth, outputHeight});
    internalSurfaceDesc.push_back(
        {"YCoCgColor", reinterpret_cast<void *>(GL_R32UI), inputWidth, inputHeight});
    internalSurfaceDesc.push_back(
        {"MotionDepthClipAlphaBuffer", reinterpret_cast<void *>(GL_RGBA16F), inputWidth,
         inputHeight});

    if (algoMode == THREE_PASS_MODE || algoMode == SWITCH_PASS_MODE) {
        internalSurfaceDesc.push_back(
            {"MotionDepthAlphaBuffer", reinterpret_cast<void *>(GL_RGBA16F), inputWidth,
             inputHeight});
        internalSurfaceDesc.push_back(
            {"PrevLumaHistory", reinterpret_cast<void *>(GL_R32UI), inputWidth, inputHeight});
        internalSurfaceDesc.push_back(
            {"LumaHistory", reinterpret_cast<void *>(GL_R32UI), inputWidth, inputHeight});
    }

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

    glGenSamplers(1, &m_LinearRepeat);
    glSamplerParameteri(m_LinearRepeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glSamplerParameteri(m_LinearRepeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glSamplerParameteri(m_LinearRepeat, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glSamplerParameteri(m_LinearRepeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glSamplerParameteri(m_LinearRepeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenSamplers(1, &m_LinearClamp);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glSamplerParameteri(m_LinearClamp, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    m_Sgsr2Ubo = glutil::Buffer::create(GL_UNIFORM_BUFFER, sizeof(Sgsr2Ubo), GL_DYNAMIC_DRAW);

    if (algoMode == THREE_PASS_MODE || algoMode == SWITCH_PASS_MODE) {
        CreatePipeline(SGSR2_PASS_CONVERT, &pipelineConvert);
        CreatePipeline(SGSR2_PASS_ACTIVATE, &pipelineActivate);
        CreatePipeline(SGSR2_PASS_UPSCALE, &pipelineUpscale);
    }
    if (algoMode == TWO_PASS_MODE || algoMode == SWITCH_PASS_MODE) {
        CreatePipeline(SGSR2_PASS_CONVERT2PASS, &pipelineConvert2Pass);
        CreatePipeline(SGSR2_PASS_UPSCALE2PASS, &pipelineUpscale2Pass);
    }

}

void SGSR2_Core::ScheduleJobCompute(const Pipeline pipeline,
                                    ComputeJobDescription *computeJobDescription,
                                    uint32_t dispatchX, uint32_t dispatchY,
                                    uint32_t dispatchZ) {
    computeJobDescription->pipeline = pipeline;
    computeJobDescription->dimensions[0] = dispatchX;
    computeJobDescription->dimensions[1] = dispatchY;
    computeJobDescription->dimensions[2] = dispatchZ;
}

void SGSR2_Core::ExecuteJobCompute(SgsrPass pass, ComputeJobDescription *computeJobDescription) {
    glUseProgram(computeJobDescription->pipeline.program);
    int pingpong =
        m_FrameIdx % 2;   //Use pingpong to switch between previous and current texture

    switch (pass) {
        case SGSR2_PASS_CONVERT: {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_OPAQUE_COLOR].resource);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR].resource);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH].resource);
            glBindSampler(2, m_NearestClamp);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY].resource);

            glBindImageTexture(0,
                               resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_ALPHA_BUFFER].resource,
                               0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

            glBindImageTexture(1, resources[SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR].resource, 0,
                               GL_FALSE,
                               0, GL_WRITE_ONLY, GL_R32UI);

            m_Sgsr2Ubo.bindBase(0);
            break;
        }
        case SGSR2_PASS_ACTIVATE: {
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_PREV_LUMA_HISTORY +
                                    pingpong].resource);
            glBindSampler(4, m_NearestClamp);

            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_ALPHA_BUFFER].resource);
            glBindSampler(5, m_NearestClamp);

            glActiveTexture(GL_TEXTURE6);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR].resource);
            glBindSampler(6, m_NearestClamp);

            glBindImageTexture(0,
                               resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER].resource,
                               0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glBindImageTexture(1, resources[SGSR2_RESOURCE_IDENTIFIER_PREV_LUMA_HISTORY +
                                            (1 - pingpong)].resource, 0, GL_FALSE,
                               0, GL_WRITE_ONLY, GL_R32UI);

            m_Sgsr2Ubo.bindBase(0);
            break;
        }
        case SGSR2_PASS_UPSCALE:
        case SGSR2_PASS_UPSCALE2PASS: {
            glActiveTexture(GL_TEXTURE7);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_PREV_HISTORY_OUTPUT +
                                    pingpong].resource);
            glBindSampler(7, m_LinearClamp);

            glActiveTexture(GL_TEXTURE8);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER].resource);
            glBindSampler(8, m_LinearClamp);

            glActiveTexture(GL_TEXTURE9);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR].resource);
            glBindSampler(9, m_NearestClamp);

            glBindImageTexture(0, resources[SGSR2_RESOURCE_IDENTIFIER_UPSCALED_OUTPUT].resource,
                               0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
            glBindImageTexture(1, resources[SGSR2_RESOURCE_IDENTIFIER_PREV_HISTORY_OUTPUT +
                                            (1 - pingpong)].resource, 0, GL_FALSE,
                               0, GL_WRITE_ONLY, GL_RGBA16F);
            // Bind ubo
            m_Sgsr2Ubo.bindBase(0);
            break;
        }
        case SGSR2_PASS_CONVERT2PASS: {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR].resource);

            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH].resource);
            glBindSampler(2, m_NearestClamp);

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D,
                          resources[SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY].resource);

            glBindImageTexture(0,
                               resources[SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER].resource,
                               0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

            glBindImageTexture(1, resources[SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR].resource, 0,
                               GL_FALSE,
                               0, GL_WRITE_ONLY, GL_R32UI);

            // Bind ubo
            m_Sgsr2Ubo.bindBase(0);
            break;
        }

        default:
            break;
    }

    glMemoryBarrier(GL_ALL_BARRIER_BITS);
    glDispatchCompute(computeJobDescription->dimensions[0],
                      computeJobDescription->dimensions[1],
                      computeJobDescription->dimensions[2]);
    glMemoryBarrier(GL_ALL_BARRIER_BITS);
}

void SGSR2_Core::ExecuteSGSR2(DispatchDescription *description) {
    Sgsr2Ubo u{};
    u.renderSize.x = description->renderSize[0];
    u.renderSize.y = description->renderSize[1];
    u.displaySize.x = description->displaySize[0];
    u.displaySize.y = description->displaySize[1];
    u.renderSizeRcp.x = description->renderSizeRcp[0];
    u.renderSizeRcp.y = description->renderSizeRcp[1];
    u.displaySizeRcp.x = description->displaySizeRcp[0];
    u.displaySizeRcp.y = description->displaySizeRcp[1];
    u.jitterOffset.x = description->jitterOffset[0];
    u.jitterOffset.y = description->jitterOffset[1];
    memcpy(u.clipToPrevClip, description->clipToPrevClip, 16 * sizeof(float));
    u.preExposure = description->preExposure;
    u.cameraFovAngleHor = description->cameraFovAngleHor;
    u.cameraNear = description->cameraNear;
    u.MinLerpContribution = description->MinLerpContribution;
    u.bSameCamera = description->sameCameraFrmNum;
    u.reset = description->reset;
    uint8_t algoMode = description->mode;

    RegisterResource(&description->opaqueColorTexture,
                     SGSR2_RESOURCE_IDENTIFIER_INPUT_OPAQUE_COLOR);
    RegisterResource(&description->colorTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR);
    RegisterResource(&description->depthTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH);
    RegisterResource(&description->velocityTexture, SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY);

    //check algorithm mode
    if (!(algoMode == TWO_PASS_MODE || algoMode == THREE_PASS_MODE)) {
        SGSR2_LOGE("Invalid dispatching algorithm mode!");
        return;
    }

    if (m_Mode != SWITCH_PASS_MODE && algoMode != m_Mode) {
        SGSR2_LOGE("algorithm mode when dispatching is not compatible with initializing!");
        return;
    }

    m_Sgsr2Ubo.subData(&u);
    glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

    //Calculate dispatch size
    uint32_t dx = divideRoundUp(description->renderSize[0], 8);
    uint32_t dy = divideRoundUp(description->renderSize[1], 8);

    if (algoMode == THREE_PASS_MODE) {
        ScheduleJobCompute(pipelineConvert, &computeJobDescriptionConvert, dx, dy, 1);
        ScheduleJobCompute(pipelineActivate, &computeJobDescriptionActivate, dx, dy, 1);
        dx = divideRoundUp(description->displaySize[0], 8);
        dy = divideRoundUp(description->displaySize[1], 8);
        ScheduleJobCompute(pipelineUpscale, &computeJobDescriptionUpscale, dx, dy, 1);
        ExecuteJobCompute(SGSR2_PASS_CONVERT, &computeJobDescriptionConvert);
        ExecuteJobCompute(SGSR2_PASS_ACTIVATE, &computeJobDescriptionActivate);
        ExecuteJobCompute(SGSR2_PASS_UPSCALE, &computeJobDescriptionUpscale);
    } else if (algoMode == TWO_PASS_MODE) {
        ScheduleJobCompute(pipelineConvert2Pass, &computeJobDescriptionConvert, dx, dy, 1);
        dx = divideRoundUp(description->displaySize[0], 8);
        dy = divideRoundUp(description->displaySize[1], 8);
        ScheduleJobCompute(pipelineUpscale2Pass, &computeJobDescriptionUpscale, dx, dy, 1);
        ExecuteJobCompute(SGSR2_PASS_CONVERT2PASS, &computeJobDescriptionConvert);
        ExecuteJobCompute(SGSR2_PASS_UPSCALE2PASS, &computeJobDescriptionUpscale);
    }
    description->outputTexture = resources[SGSR2_RESOURCE_IDENTIFIER_UPSCALED_OUTPUT];
    m_FrameIdx++;

}

void SGSR2_Core::Destroy() {
    DestroyPipelines();
    DestroyResources();
}