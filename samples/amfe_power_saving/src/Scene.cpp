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
#include "OpenGLES/FrmShader.h"
#include "OpenGLES/FrmUtilsGLES.h"


#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif


//requires file write permissions
#define ADRENO_DEBUG_BACK_BUFFER_FRAME_WRITE_TO_TGA 0
#define ADRENO_DEBUG_EXTRAPOLATE_FRAME_WRITE_TO_TGA 0


//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    CSample*const sample = new CSample("AMFE Power Saving");
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
        "#version 300 es                                                    \n"
        "in vec4 g_vVertex;                                                 \n"
        "in vec2 g_vVertexTex;                                              \n"
        "in vec4 g_vVertexNormal;                                           \n"
        "uniform mat4   g_modelToClip;                                      \n"
        "uniform mat4   g_modelRotationLocal;                               \n"
        "out vec2   g_vTexCoord;                                            \n"
        "out vec4   g_vNormal;                                              \n"
        "                                                                   \n"
        "void main()                                                        \n"
        "{                                                                  \n"
        "    gl_Position  = g_modelToClip*vec4( g_vVertex.x, g_vVertex.y,   \n"
        "                                       g_vVertex.z, g_vVertex.w ); \n"
        "    g_vTexCoord = g_vVertexTex;                                    \n"
        "    g_vNormal = g_modelRotationLocal*g_vVertexNormal;              \n"
        "}                                                                  \n";

    m_mainFragmentShader =
        "#version 300 es                                                    \n"
        "out vec4 fragColor;                                                \n"
        "#ifdef GL_FRAGMENT_PRECISION_HIGH                                  \n"
        "   precision highp float;                                          \n"
        "#else                                                              \n"
        "   precision mediump float;                                        \n"
        "#endif                                                             \n"
        "                                                                   \n"
        "uniform sampler2D g_Texture;                                       \n"
        "in vec2      g_vTexCoord;                                          \n"
        "in vec4      g_vNormal;                                            \n"
        "void main()                                                        \n"
        "{                                                                  \n"
        "    fragColor = texture( g_Texture, g_vTexCoord );                 \n"
        "}                                                                  \n";

    //calculates "full screen triangle" which is just large enough to cover the entire screen, with UV coordinates that exactly cover the screen
    m_fullscreenTriangleVertexShader =
            "#version 300 es                                                \n"
            "out   vec2 g_vTexCoord;                                        \n"
            "                                                               \n"
            "void main()                                                    \n"
            "{                                                              \n"
            "    gl_Position.x = -1.0 + float((gl_VertexID & 1) << 2);      \n"
            "    gl_Position.y = -1.0 + float((gl_VertexID & 2) << 1);      \n"
            "    gl_Position.z = 0.0;                                       \n"
            "    gl_Position.w = 1.0;                                       \n"
            "                                                               \n"
            "    g_vTexCoord.x = (gl_Position.x+1.0)*0.5;                   \n"
            "    g_vTexCoord.y = (gl_Position.y+1.0)*0.5;                   \n"
            "}                                                              \n";

    m_fullscreenCopyFragmentShader =
            "#version 300 es                                                \n"
            "out vec4 fragColor;                                            \n"
            "#ifdef GL_FRAGMENT_PRECISION_HIGH                              \n"
            "   precision highp float;                                      \n"
            "#else                                                          \n"
            "   precision mediump float;                                    \n"
            "#endif                                                         \n"
            "uniform sampler2D g_Texture;                                   \n"
            "in vec2      g_vTexCoord;                                      \n"
            "                                                               \n"
            "void main()                                                    \n"
            "{                                                              \n"
            "    fragColor = texture( g_Texture, g_vTexCoord );             \n"
            "}                                                              \n";


    return TRUE;
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

    const char*const GL_QCOM_frame_extrapolation_cStr = "GL_QCOM_frame_extrapolation";
    const bool GL_QCOM_frame_extrapolation_supported = FrmGLExtensionSupported(GL_QCOM_frame_extrapolation_cStr);
    if(!GL_QCOM_frame_extrapolation_supported)
    {
        LOGI("Extension %s ia not supported; sample can't run", GL_QCOM_frame_extrapolation_cStr);
        ADRENO_ASSERT(GL_QCOM_frame_extrapolation_supported, __FILE__, __LINE__);
        return FALSE;//sample can't run
    }
    m_glExtrapolateTex2DQCOM = (PFNGLQCOMFRAMEEXTRAPOLATION)eglGetProcAddress("glExtrapolateTex2DQCOM");
    ADRENO_ASSERT(m_glExtrapolateTex2DQCOM, __FILE__, __LINE__);


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

    const size_t frameRenderedResourcesNum = m_offscreenFramebufferTexturesColor.Size();
    assert(frameRenderedResourcesNum == m_offscreenFramebuffers.Size());
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_WIDTH, &m_eglSurfaceWidth);
    eglQuerySurface(m_eglDisplay, m_eglSurface, EGL_HEIGHT, &m_eglSurfaceHeight);

    //create three frame-buffer/texture pairs to store two source images and one output image for frame extrapolation
    const GLint textureSizedInternalFormat = GL_RGB8;
    FrmCreateTexture(
        &m_extrapolatedTexture,
        textureSizedInternalFormat,
        m_eglSurfaceWidth,
        m_eglSurfaceHeight);
    FrmCreateAndBindFramebufferToTexture(&m_extrapolatedFramebuffer, m_extrapolatedTexture);
    for (size_t i = 0; i < frameRenderedResourcesNum; ++i)
    {
        auto& backBufferTextureColor = m_offscreenFramebufferTexturesColor[i];
        auto& backBufferTextureDepth = m_offscreenFramebufferTexturesDepth[i];
        FrmCreateTexture(
            &backBufferTextureColor,
            textureSizedInternalFormat,
            m_eglSurfaceWidth,
            m_eglSurfaceHeight);
        FrmCreateTexture(
            &backBufferTextureDepth,
            GL_DEPTH_COMPONENT16,
            m_eglSurfaceWidth,
            m_eglSurfaceHeight);
        FrmCreateAndBindFramebufferToTexture(&m_offscreenFramebuffers[i], backBufferTextureColor, backBufferTextureDepth);
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
        m_extrapolatingEveryOtherFrame = !m_extrapolatingEveryOtherFrame;
    }

    return;
}

static void SaveFramebufferToTGA(
    const CHAR* const tgaFilePath,
    const EGLint widthPixels,
    const EGLint heightPixels,
    const GLuint framebufferHandle)
{
    //output blitted backbuffer from one frame, assuming RGB format
    ADRENO_ASSERT(CStringNotEmpty(tgaFilePath), __FILE__, __LINE__);
    ADRENO_ASSERT(widthPixels > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(heightPixels > 0, __FILE__, __LINE__);

    static UINT8* s_readPixelsBufferRgb;
    static UINT8* s_readPixelsBufferRgba;
    if (!s_readPixelsBufferRgb)
    {
        ADRENO_ASSERT(!s_readPixelsBufferRgba, __FILE__, __LINE__);

        //rely on OS to reclaim allocated memory upon program-close
        const size_t pixelsNum = widthPixels * heightPixels;
        s_readPixelsBufferRgb = reinterpret_cast<UINT8*>(malloc(pixelsNum * 3));
        s_readPixelsBufferRgba = reinterpret_cast<UINT8*>(malloc(pixelsNum * 4));
    }

    FrmReadFramebufferCpu(
        s_readPixelsBufferRgb, 
        widthPixels, 
        heightPixels, 
        GL_RGB, 
        GL_UNSIGNED_BYTE,
        framebufferHandle);

    FrmSaveImageAsTGA_RGB_to_RGBA(
        s_readPixelsBufferRgba,
        s_readPixelsBufferRgb,
        tgaFilePath,
        widthPixels,
        heightPixels);
}

VOID CSample::OverlayTextDraw()
{
    ArraySafe<char,256> s;
    s.Snprintf("%s\n(tap screen to toggle)", m_extrapolatingEveryOtherFrame ? "Extrapolating every other frame" : "Traditionally rendering every frame");
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

    if (m_extrapolateThisFrame)
    {
        m_glExtrapolateTex2DQCOM(
            m_offscreenFramebufferTexturesColor[m_framesTraditionalRendered % 2],
            m_offscreenFramebufferTexturesColor[m_framesTraditionalRendered % 2 ? 0 : 1],
            m_extrapolatedTexture,
            .5f);//extrapolate half the "distance" between these two frames to attempt to double framerate

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);        
        ClearFast(GL_COLOR_BUFFER_BIT);
        FrmBlitAvoidGMemLoads(m_extrapolatedTexture, m_fullscreenCopyShader);

#if ADRENO_DEBUG_EXTRAPOLATE_FRAME_WRITE_TO_TGA
        if (m_framesTraditionalRendered == 10)
        {
            DebugExtrapolateFrameWriteToTGA();
        }
#endif//#if ADRENO_DEBUG_EXTRAPOLATE_FRAME_WRITE_TO_TGA

    }
    else//render frame
    {
        float timeSinceLastTraditionalRender;
        if (m_framesTraditionalRendered == 0)
        {
            timeSinceLastTraditionalRender = 0.f;
            Adreno::ClockGetTime(&m_traditionalRenderStart);
        }
        else
        {
            Adreno::ClockGetTime(&m_traditionalRenderStop);
            timeSinceLastTraditionalRender = static_cast<float>(Adreno::Seconds(m_traditionalRenderStop - m_traditionalRenderStart));
            m_traditionalRenderStart = m_traditionalRenderStop;
        }

        //LOGI("timeSinceLastTraditionalRender=%f", timeSinceLastTraditionalRender);

        //render to texture so it's saved for frame extrapolation
        const size_t framesTraditionalIndex = m_framesTraditionalRendered % 2;
        const GLuint savedBackbufferFramebufferHandle = m_offscreenFramebuffers[framesTraditionalIndex];
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, savedBackbufferFramebufferHandle);
        RenderTraditional(timeSinceLastTraditionalRender);

        //avoid unnecessary GMEMStore'ing depth or stencil data, since only the color data from this buffer will be copied to the back buffer
        const size_t attachmentsToInvalidateNum = 2;
        const GLenum attachmentsToInvalidate[attachmentsToInvalidateNum] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
        glInvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, attachmentsToInvalidateNum, &attachmentsToInvalidate[0] );

        //blit offscreen texture to backbuffer
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        ClearFast(GL_COLOR_BUFFER_BIT);
        FrmBlitAvoidGMemLoads(m_offscreenFramebufferTexturesColor[framesTraditionalIndex], m_fullscreenCopyShader);

        ++m_framesTraditionalRendered;

#if ADRENO_DEBUG_BACK_BUFFER_FRAME_WRITE_TO_TGA
        if (m_framesTraditionalRendered == 10)
        {
            SaveFramebufferToTGA(
                "sdcard/Download/SavedBackBuffer.tga",
                m_eglSurfaceWidth,
                m_eglSurfaceHeight,
                savedBackbufferFramebufferHandle);
        }
#endif
    }
    OverlayTextDraw();

    if (m_framesTraditionalRendered > 1)//after we've rendered two frames, extrapolate every other frame
    {
        m_extrapolateThisFrame = !m_extrapolateThisFrame;
    }
    if (!m_extrapolatingEveryOtherFrame)
    {
        m_extrapolateThisFrame = false;
    }
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

VOID CSample::DebugExtrapolateFrameWriteToTGA()
{
    SaveFramebufferToTGA(
        "sdcard/Download/SavedSourceBuffer0.tga",
        m_eglSurfaceWidth,
        m_eglSurfaceHeight,
        m_offscreenFramebuffers[m_framesTraditionalRendered % 2]);
    SaveFramebufferToTGA(
        "sdcard/Download/SavedSourceBuffer1.tga",
        m_eglSurfaceWidth,
        m_eglSurfaceHeight,
        m_offscreenFramebuffers[m_framesTraditionalRendered % 2 ? 0 : 1]);

    SaveFramebufferToTGA(
        "sdcard/Download/SavedExtrapolatedBuffer.tga",
        m_eglSurfaceWidth,
        m_eglSurfaceHeight,
        m_extrapolatedFramebuffer);
}