// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Scene.h"

#include <GLES3/gl31.h>
#include <string>
#include <tinygltf/examples/common/glm/glm/gtc/matrix_transform.hpp>
#include <vector>

#include "FrmArraySafe.h"
#include "FrmSpline.h"
#include "FrmUtils.h"
#include "OpenGLES/FrmResourceGLES.h"
#include "OpenGLES/FrmShader.h"
#include "OpenGLES/FrmResourceGLES.h"
#include "OpenGLES/FrmShader.h"
#include "OpenGLES/FrmUtilsGLES.h"



#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif


//requires file write permissions
#define ADRENO_DEBUG_LUMINANCE_AND_MOTION_VECTORS_WRITE_TO_TGA 0


//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    CSample*const sample = new CSample("Motion Estimation");
    sample->m_nNumStencilBufferBits = sample->m_nNumDepthBufferBits = 0;//back buffer needs only a color buffer to receive final results from offscreen render targets, so save Gpu memory
    return sample;
}


//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample(const CHAR* strName) : CFrmApplication(strName)
{
    m_windowTitle = strName;
    m_windowWidth = 765;
    m_windowHeight = 480;
    m_aspectRatio = (FLOAT)m_windowWidth / (FLOAT)m_windowHeight;

    m_mainShader = 0;
    m_vertexLoc = 0;
    m_textureLoc = 1;
    m_normalLoc = 2;
}


//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    m_mainVertexShader = 
        "#version 300 es                                                            \n"
        "in vec4 g_vVertex;                                                         \n"
        "in vec2 g_vVertexTex;                                                      \n"
        "in vec4 g_vVertexNormal;                                                   \n"
        "uniform mat4   g_modelToClip;                                              \n"
        "uniform mat4   g_modelRotationLocal;                                       \n"
        "out vec2   g_vTexCoord;                                                    \n"
        "out vec4   g_vNormal;                                                      \n"
        "                                                                           \n"
        "void main()                                                                \n"
        "{                                                                          \n"
        "    gl_Position  = g_modelToClip*vec4( g_vVertex.x, g_vVertex.y,           \n"
        "                                       g_vVertex.z, g_vVertex.w );         \n"
        "    g_vTexCoord = g_vVertexTex;                                            \n"
        "    g_vNormal = g_modelRotationLocal*g_vVertexNormal;                      \n"
        "}                                                                          \n";

    m_mainFragmentShader =
        "#version 300 es                                                            \n"
        "out vec4 fragColor;                                                        \n"
        "#ifdef GL_FRAGMENT_PRECISION_HIGH                                          \n"
        "   precision highp float;                                                  \n"
        "#else                                                                      \n"
        "   precision mediump float;                                                \n"
        "#endif                                                                     \n"
        "                                                                           \n"
        "uniform sampler2D g_Texture;                                               \n"
        "in vec2      g_vTexCoord;                                                  \n"
        "in vec4      g_vNormal;                                                    \n"
        "void main()                                                                \n"
        "{                                                                          \n"
        "    fragColor = texture( g_Texture, g_vTexCoord );                         \n"
        "}                                                                          \n";

    //calculates "full screen triangle" which is just large enough to cover the entire screen, with UV coordinates that exactly cover the screen
    m_fullscreenTriangleVertexShader =
            "#version 300 es                                                        \n"
            "out   vec2 g_vTexCoord;                                                \n"
            "                                                                       \n"
            "void main()                                                            \n"
            "{                                                                      \n"
            "    gl_Position.x = -1.0 + float((gl_VertexID & 1) << 2);              \n"
            "    gl_Position.y = -1.0 + float((gl_VertexID & 2) << 1);              \n"
            "    gl_Position.z = 0.0;                                               \n"
            "    gl_Position.w = 1.0;                                               \n"
            "                                                                       \n"
            "    g_vTexCoord.x = (gl_Position.x+1.0)*0.5;                           \n"
            "    g_vTexCoord.y = (gl_Position.y+1.0)*0.5;                           \n"
            "}                                                                      \n";

    m_rgbaToLuminanceFragmentShader =
            "#version 300 es                                                        \n"
            "out float fragColor;                                                   \n"
            "#ifdef GL_FRAGMENT_PRECISION_HIGH                                      \n"
            "   precision highp float;                                              \n"
            "#else                                                                  \n"
            "   precision mediump float;                                            \n"
            "#endif                                                                 \n"
            "uniform sampler2D g_Texture;                                           \n"
            "in vec2      g_vTexCoord;                                              \n"
            "                                                                       \n"
            "void main()                                                            \n"
            "{                                                                      \n"
            "    vec4 inColor = texture( g_Texture, g_vTexCoord );                  \n"
            "    fragColor = 0.299*inColor.r + 0.587*inColor.g + 0.114*inColor.b;   \n"
            "}                                                                      \n";

    m_fullscreenCopyFragmentShader =
            "#version 300 es                                                        \n"
            "out vec4 fragColor;                                                    \n"
            "#ifdef GL_FRAGMENT_PRECISION_HIGH                                      \n"
            "   precision highp float;                                              \n"
            "#else                                                                  \n"
            "   precision mediump float;                                            \n"
            "#endif                                                                 \n"
            "uniform sampler2D g_Texture;                                           \n"
            "in vec2      g_vTexCoord;                                              \n"
            "                                                                       \n"
            "void main()                                                            \n"
            "{                                                                      \n"
            "    fragColor = texture( g_Texture, g_vTexCoord );                     \n"
            "}                                                                      \n";
    return TRUE;
}

static EGLint RoundUpToNearestMultiple(const EGLint v, const EGLint multiple)
{
	ADRENO_ASSERT(v > 0, __FILE__, __LINE__);
	ADRENO_ASSERT(multiple > 0, __FILE__, __LINE__);
	
	return ((v + multiple-1)/multiple)*multiple;
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{    
    InitShaders();

    const size_t kAttrsNum=3;
    FRM_SHADER_ATTRIBUTE attrs[kAttrsNum];
    attrs[0].strName = "g_vVertex";
    attrs[0].nLocation = m_vertexLoc;
    attrs[1].strName = "g_vVertexTex";
    attrs[1].nLocation = m_textureLoc;
    attrs[2].strName = "g_vVertexNormal";
    attrs[2].nLocation = m_normalLoc;
    if(!FrmCompileShaderProgram(m_mainVertexShader, m_mainFragmentShader, &m_mainShader, &attrs[0], kAttrsNum))
    {
        return FALSE;
    }
    if(!FrmCompileShaderProgram(m_fullscreenTriangleVertexShader, m_fullscreenCopyFragmentShader, &m_fullscreenCopyShader))
    {
        return FALSE;
    }
    if(!FrmCompileShaderProgram(m_fullscreenTriangleVertexShader, m_rgbaToLuminanceFragmentShader, &m_rgbaToLuminanceShader))
    {
        return FALSE;
    }

    if (FALSE == m_font.Create("Tuffy16.pak"))
    {
        return FALSE;
    }
    m_font.SetScaleFactors(3.0f, 3.0f);


    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    m_modelToClipLoc = glGetUniformLocation(m_mainShader, "g_modelToClip");
    m_modelRotationLocalLoc = glGetUniformLocation(m_mainShader, "g_modelRotationLocal");

    const char*const GL_QCOM_motion_estimation_cStr = "GL_QCOM_motion_estimation";
    const bool GL_QCOM_motion_estimation_supported = FrmGLExtensionSupported(GL_QCOM_motion_estimation_cStr);
    if(!GL_QCOM_motion_estimation_supported)
    {
        LOGI("Extension %s ia not supported; sample can't run", GL_QCOM_motion_estimation_cStr);
        ADRENO_ASSERT(GL_QCOM_motion_estimation_supported, __FILE__, __LINE__);
        return FALSE;//sample can't run
    }
    m_glTexEstimateMotionQCOM = (PFNGLQCOMTEXESTIMATEMOTION)eglGetProcAddress("glTexEstimateMotionQCOM");
    ADRENO_ASSERT(m_glTexEstimateMotionQCOM, __FILE__, __LINE__);


    std::vector<Adreno::Model> adrenoModels;
    m_textureFilepaths.push_back(std::string("Textures/white_d.ktx"));//note that if support is added for normal maps, Textures/normal_default.png may become relevant
    m_baseColorDefault_MaterialId = m_textureFilepaths.size();    
    const bool gltfLoadResult = Adreno::FrmLoadGLTFModelsFromTextFile(
        &adrenoModels, 
        &m_textureFilepaths, 
        &m_meshesSortedByMaterialId, 
        "Meshes/Museum.gltf",
        m_baseColorDefault_MaterialId);
    ADRENO_ASSERT(gltfLoadResult, __FILE__, __LINE__);

    Adreno::FrmMeshesMergeIfSameMaterial(&m_adrenoModelMerged, m_meshesSortedByMaterialId);
    //non-merged adrenoModels' dynamic memory will be freed upon this function returning in their destructors

    Adreno::FrmKtxFilesLoad(&m_textureHandles, m_textureFilepaths);

    size_t indexBufferHandleIndex = 0;
    const auto& adrenoModel = m_adrenoModelMerged;
    for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex)
    {
        const Adreno::Mesh& mesh = adrenoModel.Meshes[meshIndex];

        const UINT32 meshNumSurfaces = mesh.Surfaces.NumSurfaces;
        if (meshNumSurfaces > 0)
        {
            ADRENO_ASSERT(meshNumSurfaces == 1, __FILE__, __LINE__);
        }

        const Adreno::IndexBuffer adrenoIndexBuffer = mesh.Indices;
        GLuint& indexBufferHandle = m_indexBufferHandles[indexBufferHandleIndex++];
        glGenBuffers(1, &indexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, adrenoIndexBuffer.NumIndices * sizeof(adrenoIndexBuffer.Indices[0]), &adrenoIndexBuffer.Indices[0], GL_STATIC_DRAW);
        //LOGI("adrenoIndexBuffer.NumIndices=%u;sizeof(adrenoIndexBuffer.Indices[0])=%zu;&adrenoIndexBuffer.Indices[0]=%p",
        //    adrenoIndexBuffer.NumIndices, sizeof(adrenoIndexBuffer.Indices[0]), &adrenoIndexBuffer.Indices[0]);
    }


    //query back buffer pixel dimensions
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    EGLint eglSurfaceWidth, eglSurfaceHeight;
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_WIDTH, &eglSurfaceWidth);
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_HEIGHT, &eglSurfaceHeight);

    const GLenum MOTION_ESTIMATION_SEARCH_BLOCK_X_QCOM = 0x8C90;
    const GLenum MOTION_ESTIMATION_SEARCH_BLOCK_Y_QCOM = 0x8C91;

    glGetIntegerv(MOTION_ESTIMATION_SEARCH_BLOCK_X_QCOM, &m_motionEstimationSearchBlockX);
    glGetIntegerv(MOTION_ESTIMATION_SEARCH_BLOCK_Y_QCOM, &m_motionEstimationSearchBlockY);
		
    LOGI(   "MOTION_ESTIMATION_SEARCH_BLOCK_X_QCOM=%i;MOTION_ESTIMATION_SEARCH_BLOCK_Y_QCOM=%i", 
            m_motionEstimationSearchBlockX, m_motionEstimationSearchBlockY);

	m_offscreenRenderTexturesWidth = RoundUpToNearestMultiple(eglSurfaceWidth, m_motionEstimationSearchBlockX);
	m_offscreenRenderTexturesHeight = RoundUpToNearestMultiple(eglSurfaceHeight, m_motionEstimationSearchBlockY);

    //create offscreen buffer for main scene rendering, and two frame-buffer/texture pairs for the luminance textures that calculate motion vectors
	LOGI("eglSurface=[%i,%i], m_offscreenRenderTextures=[%i,%i]", eglSurfaceWidth, eglSurfaceHeight, m_offscreenRenderTexturesWidth, m_offscreenRenderTexturesHeight);
    FrmCreateTexture(&m_motionVectorsTexture, GL_RGBA16F, MotionVectorsTextureWidthPixels(), MotionVectorsTextureHeightPixels());
    FrmCreateAndBindFramebufferToTexture(&m_motionVectorsFramebuffer, m_motionVectorsTexture);

    FrmCreateTexture(
        &m_offscreenFramebufferTextureColor,
        GL_RGB8,
        m_offscreenRenderTexturesWidth,
        m_offscreenRenderTexturesHeight);
    FrmCreateTexture(
        &m_offscreenFramebufferTextureDepth,
        GL_DEPTH_COMPONENT16,
        m_offscreenRenderTexturesWidth,
        m_offscreenRenderTexturesHeight);
    FrmCreateAndBindFramebufferToTexture(&m_offscreenFramebuffer, m_offscreenFramebufferTextureColor, m_offscreenFramebufferTextureDepth);
    const size_t luminanceTextureResourcesNum = m_luminanceTextures.Size();
    for (size_t i = 0; i < luminanceTextureResourcesNum; ++i)
    {
        auto& luminanceTextureColor = m_luminanceTextures[i];
        FrmCreateTexture(
            &luminanceTextureColor,
            GL_R8,
            m_offscreenRenderTexturesWidth,
            m_offscreenRenderTexturesHeight);
        FrmCreateAndBindFramebufferToTexture(&m_luminanceFramebuffers[i], luminanceTextureColor);
    }

    return TRUE;
}



//--------------------------------------------------------------------------------------
// Name: Resize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Resize()
{
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Destroy()
{
    glDeleteProgram(m_mainShader);
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Update()
{
    // Process input
    UINT32 pntState;
    FRMVECTOR2 newMousePose;
    m_Input.GetPointerState(&pntState, &newMousePose);

    // Toggle if user touches the screen anywhere.
    if (pntState & FRM_INPUT::POINTER_RELEASED)
    {
        m_mode = static_cast<Mode>((static_cast<int>(m_mode) + 1) % MODE_NUM);
    }

    return;
}

static void SaveRedFramebufferToTGA(
    const CHAR* const tgaFilePath,
    UINT8* const rgbaScratchBuffer,
    UINT8* const readPixelsBuffer,    
    const EGLint widthPixels,
    const EGLint heightPixels,
    const GLuint framebufferHandle)
{
    ADRENO_ASSERT(CStringNotEmpty(tgaFilePath), __FILE__, __LINE__);
    ADRENO_ASSERT(readPixelsBuffer, __FILE__, __LINE__);
    ADRENO_ASSERT(rgbaScratchBuffer, __FILE__, __LINE__);
    ADRENO_ASSERT(widthPixels > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(heightPixels > 0, __FILE__, __LINE__);

    FrmReadFramebufferCpu(readPixelsBuffer, widthPixels, heightPixels, GL_RED, GL_UNSIGNED_BYTE, framebufferHandle);
    FrmSaveImageAsTGA_R_to_RGBA(rgbaScratchBuffer, readPixelsBuffer, tgaFilePath, widthPixels, heightPixels);
}

VOID CSample::OverlayTextDraw(const float timeSinceLastRender)
{
    ArraySafe<char,256> s;
    const char* overlayTextAppend;
    const char*const regularStr = "Rendering normally";
    const char*const motionVectorsStr = "Rendering motion vectors";
    switch(m_mode)
    {
        case MODE_REGULAR:
        {
            overlayTextAppend = regularStr;
            break;
        }
        case MODE_MOTION_VECTORS:
        {
            overlayTextAppend = motionVectorsStr;
            break;
        }        
        default:
            ADRENO_ASSERT(false, __FILE__, __LINE__);
        {
            overlayTextAppend = nullptr;
            break;
        }
    }
    s.Snprintf("%s\n(tap screen to change rendering mode)\nCpu framerate=%f seconds", overlayTextAppend, timeSinceLastRender);
    m_font.DrawText(m_nWidth / 2.0f, 80, FRMCOLOR_WHITE, &s[0], FRM_FONT_CENTER);
}

static void ClearFast(const GLbitfield bufferBits)
{
    // Clear the backbuffer with "fast-transparent-black" color, helping to ensure no unnecessary (and potentially costly) GMemLoads
    ADRENO_ASSERT(bufferBits != 0, __FILE__, __LINE__);//pass all attachments to the buffer to ensure no GMemLoads; eg maximally (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(bufferBits);            
}

//--------------------------------------------------------------------------------------
// Name: Render()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Render()
{
    ADRENO_ASSERT(glGetError() == GL_NO_ERROR, __FILE__, __LINE__);

    float timeSinceLastRender;
    if (m_framesRendered == 0)
    {
        timeSinceLastRender = 0.f;
        Adreno::ClockGetTime(&m_renderStart);
    }
    else
    {
        Adreno::ClockGetTime(&m_renderStop);
        timeSinceLastRender = static_cast<float>(Adreno::Seconds(m_renderStop - m_renderStart));
        m_renderStart = m_renderStop;
    }

    //LOGI("timeSinceLastRender=%f", timeSinceLastRender);

    //render to offscreen texture first
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_offscreenFramebuffer);
    RenderTraditional(timeSinceLastRender);

    //try to avoid unnecessary GMEMStore'ing depth or stencil data, since only the color data from this buffer will be copied to the back buffer
    const size_t attachmentsToInvalidateNum = 2;
    const GLenum attachmentsToInvalidate[attachmentsToInvalidateNum] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    glInvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, attachmentsToInvalidateNum, &attachmentsToInvalidate[0] );

    //blit offscreen texture to backbuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    ClearFast(GL_COLOR_BUFFER_BIT);//try to avoid unnecessary GMEMStore's by alerting GL driver the previous color data is to be overwritten
    FrmBlitAvoidGMemLoads(m_offscreenFramebufferTextureColor, m_fullscreenCopyShader);

    //generate luminance texture from offscreen buffer
    const size_t framesIndex = m_framesRendered % 2;
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_luminanceFramebuffers[framesIndex]);
    FrmBlitAvoidGMemLoads(m_offscreenFramebufferTextureColor, m_rgbaToLuminanceShader);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);//clean up

    if(m_mode == MODE_MOTION_VECTORS && m_framesRendered > 0)
    {
        m_glTexEstimateMotionQCOM(  m_luminanceTextures[framesIndex ? 0 : 1],
                                    m_luminanceTextures[framesIndex],
                                    m_motionVectorsTexture);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        FrmBlitAvoidGMemLoads(m_motionVectorsTexture, m_fullscreenCopyShader);
    }

#if ADRENO_DEBUG_LUMINANCE_AND_MOTION_VECTORS_WRITE_TO_TGA
    if (m_framesRendered == 10)
    {
        DebugLuminanceAndMotionVectorsBuffersWriteToTGA();
    }
#endif

    OverlayTextDraw(timeSinceLastRender);
    ++m_framesRendered;
}

VOID CSample::RenderTraditional(const float targetRenderTime)
{
    glm::vec3 splinePoints[kSplinePointsNum];
    splinePoints[0] = glm::vec3(-1.f, 0.f, -1.f);
    splinePoints[1] = glm::vec3(-1.f, 0.f, 1.f);
    splinePoints[2] = glm::vec3(1.f, 0.f, 1.f);
    splinePoints[3] = glm::vec3(1.f, 0.f, -1.f);
    for (size_t i = 0; i < kSplinePointsNum; ++i)
    {
        splinePoints[i] *= 16.f;
        splinePoints[i].y = 15.f;
    }
    const float kSplinePointsNumFloat = static_cast<float>(kSplinePointsNum);
    const glm::vec3 cameraPosition = Adreno::SplineEvaluate(m_splineT, &splinePoints[0], kSplinePointsNum);
    const glm::vec3 cameraPositionLookahead =
        Adreno::SplineEvaluate(Adreno::SplineTIncrement(m_splineT, .0625f, kSplinePointsNumFloat), &splinePoints[0], kSplinePointsNum);
    m_splineT = Adreno::SplineTIncrement(m_splineT, targetRenderTime * .2f, kSplinePointsNumFloat);


    ClearFast(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(m_mainShader);

    const glm::vec3 worldUp(0.f, 1.f, 0.f);
    const glm::mat4 localToWorld = glm::rotate(glm::mat4(), glm::radians(270.f), worldUp);
    const glm::mat4 modelToClip =
        glm::perspective(glm::radians(90.f), 1.f / m_aspectRatio, .4f, 90.f) *
        glm::lookAt(cameraPosition, cameraPositionLookahead, worldUp) *
        glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.f)) *
        localToWorld;
    glUniformMatrix4fv(m_modelToClipLoc, 1, GL_FALSE, &modelToClip[0][0]);
    glUniformMatrix4fv(m_modelRotationLocalLoc, 1, GL_FALSE, &localToWorld[0][0]);


    size_t indexBufferIndex = 0;
    const auto& adrenoModel = m_adrenoModelMerged;
    for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex)
    {
        // Draw the model
        const Adreno::Mesh& adrenoMesh = adrenoModel.Meshes[meshIndex];
        const Adreno::MeshSurfaceArray& adrenoMeshSurfaces = adrenoMesh.Surfaces;

        if (adrenoMeshSurfaces.NumSurfaces == 1)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureHandles[adrenoMeshSurfaces.SurfaceGet(0)->MaterialId]);
        }

        const Adreno::VertexFormat& adrenoVertexFormat = adrenoMesh.Vertices.Format;
        for (   INT32 adrenoVertexFormatPropertiesIndex = 0; 
                adrenoVertexFormatPropertiesIndex < adrenoVertexFormat.NumProperties; 
                ++adrenoVertexFormatPropertiesIndex)
        {
            const Adreno::VertexProperty& vertexProperty = adrenoVertexFormat.Properties[adrenoVertexFormatPropertiesIndex];
            if (vertexProperty.Valid())
            {
                const char* const vertexPropertyName = &vertexProperty.Usage.Name[0];
                const GLuint doNotHandle = UINT32_MAX;
                GLuint loc = doNotHandle;
                //LOGI(   "%zu/%s:VertexPropertyType=%i,Offset=%u,Size=%u,NumValues=%u,IsNormalized=%i",
                //        adrenoVertexFormatPropertiesIndex, &vertexProperty.Usage.Name[0], vertexProperty.Type, vertexProperty.Offset, vertexProperty.Size(), vertexProperty.NumValues(), vertexProperty.IsNormalized());
                if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_POSITION))
                {
                    loc = m_vertexLoc;
                }
                else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_NORMAL))
                {
                    loc = m_normalLoc;
                }
                else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_TEXCOORD_0))
                {
                    loc = m_textureLoc;
                }
                //don't handle other vertex attributes yet

                if (loc != doNotHandle)
                {
                    glVertexAttribPointer(
                        loc,
                        vertexProperty.NumValues(),
                        GL_FLOAT,
                        0,
                        adrenoVertexFormat.Stride,
                        &adrenoMesh.Vertices.Buffer[vertexProperty.Offset]);
                    glEnableVertexAttribArray(loc);
                }
            }
        }
        // Draw mesh
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandles[indexBufferIndex++]);
        glDrawElements(
            GL_TRIANGLES,
            adrenoMesh.Indices.NumIndices,
            GL_UNSIGNED_INT,
            NULL);

        glDisableVertexAttribArray(m_vertexLoc);
        glDisableVertexAttribArray(m_textureLoc);
        glDisableVertexAttribArray(m_normalLoc);
    }
}

static float Sign(const float f)
{
    return 1.f ? f >= 0.f : -1.f;
}
static UINT8 HalfPrecisionFloatToUnsignedByte(const UINT16 halfPrecisionFloat)
{
    if(halfPrecisionFloat == 0)
    {
        return 0;//map exact zero to exact zero
    }
    else
    {
        const UINT32 exponentBiasMovedToHalfPrecisionExponentLowestBit = (127-15) << 10;//convert exponent bias from 16bit to 32bit (15 bias to 127 bias), and shift this bias offset so it affects only the 16bit exponent bits
        const UINT32 float32Uint =  ((halfPrecisionFloat & 0x8000) << 16) | //sign bit
                                    (((halfPrecisionFloat & 0x7C00) + exponentBiasMovedToHalfPrecisionExponentLowestBit) << 13) | //5 exponent bits, shifted to the lowest 5 bit positions in 32bit exponent
                                    ((halfPrecisionFloat & 0x03FF) << 13);  //10 mantissa/significand bits shifted to the highest 10 bit positions in 32bit mantissa/significand
        const float f = *reinterpret_cast<const float*>(&float32Uint);
        const float floatMaxHalfPrecision = 25.f;//IEEE specification allows up to +/-65504.f, but this sample should rarely, if ever, see values higher than this unless framerate is significantly below 60Hz
        //LOGI("f=%f from float32Uint=0x%x from halfPrecisionFloat=0x%x", f, float32Uint, halfPrecisionFloat);
        ADRENO_ASSERT(fabs(f) <= 65504.f, __FILE__, __LINE__);

        const UINT8 halfUnsignedByteMax = 127;
        return halfUnsignedByteMax + static_cast<UINT8>(halfUnsignedByteMax*(f/floatMaxHalfPrecision) + .5f*Sign(f));//don't explicitly handle 16bit denormals; this algorithm maps them to 32bit values that are close enough to zero for this purpose
    }
}
VOID CSample::DebugLuminanceAndMotionVectorsBuffersWriteToTGA()
{
    const size_t kBuffersNum = 2;
    static UINT8* s_readPixelsBuffers[kBuffersNum];
    if (!s_readPixelsBuffers[0])
    {
        for(size_t buffersIndex=0; buffersIndex < kBuffersNum; ++buffersIndex)
        {
            //rely on OS to reclaim allocated memory upon program-close
            s_readPixelsBuffers[buffersIndex] = reinterpret_cast<UINT8*>(malloc(m_offscreenRenderTexturesWidth * m_offscreenRenderTexturesHeight * 8));//RGBA16F pixel-size at most
        }
    }
    SaveRedFramebufferToTGA(
        "sdcard/Download/SavedLuminanceBuffer0.tga",
        s_readPixelsBuffers[0],
        s_readPixelsBuffers[1],
        m_offscreenRenderTexturesWidth,
        m_offscreenRenderTexturesHeight,
        m_luminanceFramebuffers[m_framesRendered % 2]);

    SaveRedFramebufferToTGA(
        "sdcard/Download/SavedLuminanceBuffer1.tga",
        s_readPixelsBuffers[0],
        s_readPixelsBuffers[1],
        m_offscreenRenderTexturesWidth,
        m_offscreenRenderTexturesHeight,
        m_luminanceFramebuffers[m_framesRendered % 2 ? 0 : 1]);


    //save motion vector texture
    UINT8*const rgbaFloatPixelsBuffer = s_readPixelsBuffers[0];
    const GLint motionVectorsTextureWidthPixels = MotionVectorsTextureWidthPixels();
    const GLint motionVectorsTextureHeightPixels = MotionVectorsTextureHeightPixels();
    FrmReadFramebufferCpu(
        rgbaFloatPixelsBuffer, 
        motionVectorsTextureWidthPixels, 
        motionVectorsTextureHeightPixels, 
        GL_RGBA, 
        GL_HALF_FLOAT,
        m_motionVectorsFramebuffer);

    UINT8*const rgbaPixelsBuffer = s_readPixelsBuffers[1];
    UINT8* rgbaPixel = rgbaPixelsBuffer;
    const size_t pixelChannelsStride = 2;
    const size_t pixelsNum = motionVectorsTextureWidthPixels*motionVectorsTextureHeightPixels;
    for (size_t pixelsIndex = 0; pixelsIndex < pixelsNum; ++pixelsIndex)
    {
        const UINT8* const rgbaFloatPixel = &rgbaFloatPixelsBuffer[pixelsIndex * 8];
        rgbaPixel[0] = 0;                                                                                                           //blue input = undefined, output zero
        rgbaPixel[1] = HalfPrecisionFloatToUnsignedByte(*reinterpret_cast<const UINT16*>(&rgbaFloatPixel[1*pixelChannelsStride]));  //green input = vertical movement component; output scaled value
        rgbaPixel[2] = HalfPrecisionFloatToUnsignedByte(*reinterpret_cast<const UINT16*>(&rgbaFloatPixel[0*pixelChannelsStride]));  //red input = horizontal movement component; output scaled value
        rgbaPixel[3] = 255;                                                                                                         //alpha input = undefined, output max

        rgbaPixel += 4;
    }

    const BOOL saveImageAsTGAResult = FrmSaveImageAsTGA(
        "sdcard/Download/SavedMotionVectorsBuffer.tga",
        motionVectorsTextureWidthPixels, 
        motionVectorsTextureHeightPixels,
        reinterpret_cast<UINT32*>(rgbaPixelsBuffer));
    ADRENO_ASSERT(saveImageAsTGAResult, __FILE__, __LINE__);
}
