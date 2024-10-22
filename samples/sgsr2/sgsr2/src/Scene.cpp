// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Scene.h"

#include "FrmSpline.h"
#include "FrmUtils.h"
#include "FrmArraySafe.h"
#include <GLES3/gl31.h>
#include <OpenGLES/FrmShader.h>
#include <string>
#include <tinygltf/examples/common/glm/glm/gtc/matrix_transform.hpp>
#include <vector>


#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif

extern AAssetManager* g_pAssetManager;

static bool isCameraStill(glm::mat4 curVP, glm::mat4 prevVP, float threshold=1e-5){
    float vpDiff = 0;
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            vpDiff += abs(curVP[i][j] - prevVP[i][j]);
        }
    }
    if (vpDiff < threshold) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------
// Name: function Halton()
// Desc: Generate jitter offset
//--------------------------------------------------------------------------------------
static float Halton(int32_t Index, int32_t Base) {
    float Result = 0.0f;
    float InvBase = 1.0f / Base;
    float Fraction = InvBase;
    while (Index > 0) {
        Result += (Index % Base) * Fraction;
        Index /= Base;
        Fraction *= InvBase;
    }
    return Result;
}

//--------------------------------------------------------------------------------------
// Name: CFrmMainFrameBufferObject()
// Desc: FBO constructor
//--------------------------------------------------------------------------------------
CFrmMainFrameBufferObject::CFrmMainFrameBufferObject()
{
    m_hColorTexture = 0;
    m_hDepthTexture = 0;
    m_hFrameBuffer = 0;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmMainFrameBufferObject()
// Desc: FBO destructor
//--------------------------------------------------------------------------------------
CFrmMainFrameBufferObject::~CFrmMainFrameBufferObject()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: Creates objects for rendering to an FBO
//--------------------------------------------------------------------------------------
BOOL CFrmMainFrameBufferObject::Create( UINT32 nWidth, UINT32 nHeight)
{
    Destroy();
    // Save values so we can restore the viewport when finished rendering to the FBO
    glGetIntegerv( GL_VIEWPORT, (GLint*)&m_PreviousViewport );

    m_nWidth  = nWidth;
    m_nHeight = nHeight;
    glGenFramebuffers(1, &m_hFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, m_hFrameBuffer);

    // Create an offscreen texture
    FrmCreateTexture(&m_hColorTexture, GL_RGBA16F, m_nWidth, m_nHeight);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hColorTexture, 0 );

    FrmCreateTexture(&m_hDepthTexture, GL_DEPTH_COMPONENT24, m_nWidth, m_nHeight);
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_hDepthTexture, 0 );

    GLenum glAttachments[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, glAttachments);

    if( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus( GL_FRAMEBUFFER ) )
        return FALSE;

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: Destroys objects used to render to an FBO
//--------------------------------------------------------------------------------------
VOID CFrmMainFrameBufferObject::Destroy()
{
    glDeleteTextures( 1, &m_hColorTexture );
    glDeleteTextures( 1, &m_hDepthTexture );
    glDeleteFramebuffers( 1, &m_hFrameBuffer );
}


//--------------------------------------------------------------------------------------
// Name: Begin()
// Desc: Binds objects for rendering to an FBO
//--------------------------------------------------------------------------------------
VOID CFrmMainFrameBufferObject::Begin()
{
    glBindFramebuffer( GL_FRAMEBUFFER, m_hFrameBuffer );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_hDepthTexture, 0 );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_hColorTexture, 0 );
    glViewport( 0, 0, m_nWidth, m_nHeight );
}


//--------------------------------------------------------------------------------------
// Name: End()
// Desc: Ends the rendering to an FBO
//--------------------------------------------------------------------------------------
VOID CFrmMainFrameBufferObject::End()
{
    // Restore rendering the previous framebuffer
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}


//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    CSample*const sample = new CSample("SGSR2.0");
    sample->m_nNumStencilBufferBits = 0;//back buffer doesn't perform stencil operations, so save Gpu memory
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

    m_shaderProgram = 0;
    m_vertexLoc = 0;
    m_textureLoc = 1;
    m_normalLoc = 2;

    m_compShaderProgram = 0;
    m_vertexPos = 0;
    m_vertexTex = 1;

    m_cameraNear = 0.4f;
    m_cameraFar = 90.f;
    m_verticalFOV = 90.f;
    m_cameraPosition = glm::vec3(0, 0, -1);
    m_cameraPositionLookahead = glm::vec3(0, 0, -1);

    //*** SGSR2 related ***//
    m_upscaleFactor = 1.5;   //SGSR Quality Mode
    m_jitterIndex = 0;
}


//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    m_vertexShaderProgram = 
        "attribute vec4 g_vVertex;                                           \n"
        "attribute vec2 g_vVertexTex;                                        \n"
        "attribute vec4 g_vVertexNormal;                                     \n"
        "uniform mat4   g_modelToClip;                                       \n"
        "uniform mat4   g_modelRotationLocal;                                \n"
        "varying vec2   g_vTexCoord;                                         \n"
        "varying vec4   g_vNormal;                                           \n"
        "                                                                    \n"
        "void main()                                                         \n"
        "{                                                                   \n"
        "    gl_Position  = g_modelToClip*vec4( g_vVertex.x, g_vVertex.y,    \n"
        "                                       g_vVertex.z, g_vVertex.w );  \n"
        "    g_vTexCoord = g_vVertexTex;                                     \n"
        "    g_vNormal = g_modelRotationLocal*g_vVertexNormal;               \n"
        "}                                                                   \n";

    m_fragmentShaderProgram =
        "#ifdef GL_FRAGMENT_PRECISION_HIGH                                   \n"
        "   precision highp float;                                           \n"
        "#else                                                               \n"
        "   precision mediump float;                                         \n"
        "#endif                                                              \n"
        "                                                                    \n"
        "uniform sampler2D g_Texture;                                        \n"
        "varying vec2      g_vTexCoord;                                      \n"
        "varying vec4      g_vNormal;                                        \n"
        "void main()                                                         \n"
        "{                                                                   \n"
        "    gl_FragColor = texture2D( g_Texture, g_vTexCoord );             \n"
        "}                                                                   \n";

    m_compFragSource =
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

    m_compVertexSource =
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

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: PrepareSGSR()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::PrepareSGSR()
{
    //*** Initialize the velocity texture for SGSR2.0 ***//
    GLint textureSizedInternalFormat = GL_RGBA8;
    m_velocity.m_nWidth = 1;
    m_velocity.m_nHeight = 1;
    FrmCreateTexture(
        &(m_velocity.m_hTextureHandle),
        textureSizedInternalFormat,
        m_velocity.m_nWidth,
        m_velocity.m_nHeight);
    glBindTexture(GL_TEXTURE_2D, m_velocity.m_hTextureHandle);
    //since we don't have moving objects in this scene, set velocity to be all-zero.
    uint32_t size = m_velocity.m_nWidth * m_velocity.m_nHeight * 4; //GL_RGBA8 is 4 bytes
    unsigned char* initData = new unsigned char [size]();
    memset(initData, 0, size);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_velocity.m_nWidth, m_velocity.m_nHeight, GL_RGBA, GL_UNSIGNED_BYTE, initData);
    delete [] initData;

    //*** Prepare jitter positions ***//
    for (int32_t idx = 0; idx < m_jitterPositions.size(); idx++) {
        float x = Halton(idx + 1, 2) - 0.5f;
        float y = Halton(idx + 1, 3) - 0.5f;
        m_jitterPositions[idx] = glm::vec2{x, y};
    }

    //*** Create and initialize SGSR2.0 ***//
    m_outputColor.m_hTextureHandle = GL_NONE;
    uint32_t displayWidth = float(m_sgsrRenderSize.x) * m_upscaleFactor + 0.5;
    uint32_t displayHeight = float(m_sgsrRenderSize.y) * m_upscaleFactor + 0.5;
    m_sgsr2CoreComp.Init(m_sgsrRenderSize.x, m_sgsrRenderSize.y, displayWidth,displayHeight, SWITCH_PASS_MODE);
    m_sgsr2CoreFrag.Init(m_sgsrRenderSize.x, m_sgsrRenderSize.y, displayWidth,displayHeight);
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
    m_sgsrRenderSize = {720, 1600};
    m_windowWidth = m_nWidth;
    m_windowHeight = m_nHeight;
    InitShaders();
    const size_t kAttrsNum=3;
    FRM_SHADER_ATTRIBUTE attrs[kAttrsNum];
    attrs[0].strName = "g_vVertex";
    attrs[0].nLocation = m_vertexLoc;
    attrs[1].strName = "g_vVertexTex";
    attrs[1].nLocation = m_textureLoc;
    attrs[2].strName = "g_vVertexNormal";
    attrs[2].nLocation = m_normalLoc;
    if(!FrmCompileShaderProgram(m_vertexShaderProgram, m_fragmentShaderProgram, &m_shaderProgram, &attrs[0], kAttrsNum))
    {
        return FALSE;
    }

    if(!FrmCompileShaderProgram(m_compVertexSource, m_compFragSource, &m_compShaderProgram))
    {
        return FALSE;
    }

    //*** Add text UI indicating SGSR2.0 on or off ***//
    BOOL fontCreateRes = m_font.Create("Tuffy16.pak");
    ADRENO_ASSERT(fontCreateRes, __FILE__, __LINE__);
    m_font.SetScaleFactors(3.0f, 3.0f);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    m_modelToClipLoc = glGetUniformLocation(m_shaderProgram, "g_modelToClip");
    m_modelRotationLocalLoc = glGetUniformLocation(m_shaderProgram, "g_modelRotationLocal");

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
        //LOGI(   "adrenoIndexBuffer.NumIndices=%u;sizeof(adrenoIndexBuffer.Indices[0])=%zu;&adrenoIndexBuffer.Indices[0]=%p",
        //        adrenoIndexBuffer.NumIndices, sizeof(adrenoIndexBuffer.Indices[0]), &adrenoIndexBuffer.Indices[0]);
    }

    bool res = m_frameBuffer.Create(m_sgsrRenderSize.x, m_sgsrRenderSize.y);
    ADRENO_ASSERT(res == true, __FILE__, __LINE__);
    PrepareSGSR();

    ADRENO_ASSERT(glGetError() == GL_NO_ERROR, __FILE__, __LINE__);
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
    glDeleteProgram(m_shaderProgram);
    glDeleteProgram(m_compShaderProgram);
    m_frameBuffer.Destroy();
    m_sgsr2CoreComp.Destroy();
    m_sgsr2CoreFrag.Destroy();
    m_font.Destroy();
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
    if(pntState & FRM_INPUT::POINTER_RELEASED){
        if (newMousePose.x < 0.5)         //Tap left screen to switch algorithm mode
        {
            if(m_mode == SGSR_OFF)
            {
                m_mode = SGSR_THREE_PASS;
            }else if(m_mode == SGSR_THREE_PASS){
                m_mode = SGSR_TWO_PASS;
            }else if(m_mode == SGSR_TWO_PASS){
                m_mode = SGSR_TWO_PASS_FRAG;
            }else if(m_mode == SGSR_TWO_PASS_FRAG){
			    m_mode = SGSR_OFF;
			}
            m_isCameraCut = true;       //since camera is moving constantly, it's a camera-cut in view of sgsr2 everytime switch back to sgsr2.
        }else if(newMousePose.x > 0.5){           //Tap right screen to switch camera movement
            m_cameraStatus = !m_cameraStatus;
        }
    }
    return;
}


//--------------------------------------------------------------------------------------
// Name: OverlayTextDraw()
// Desc: Draw text UI
//--------------------------------------------------------------------------------------
VOID CSample::OverlayTextDraw()
{
    ArraySafe<char,256> s;
    if(m_mode == SGSR_TWO_PASS)
    {
        s.Snprintf("%s", "SGSR_2Pass");
    }else if(m_mode == SGSR_TWO_PASS_FRAG){
        s.Snprintf("%s", "SGSR_2Pass_Frag");
    }else if(m_mode == SGSR_THREE_PASS){
        s.Snprintf("%s", "SGSR_3Pass");
    }else{
        s.Snprintf("%s", "SGSR_OFF");
    }
    m_font.DrawText(m_nWidth / 3.0f, 20, FRMCOLOR_WHITE, &s[0], FRM_FONT_CENTER);
}


//--------------------------------------------------------------------------------------
// Name: CameraUpdate()
// Desc: Update camera positions and lookahead
//--------------------------------------------------------------------------------------
VOID CSample::CameraUpdate()
{
    float timeSinceLastTraditionalRender;
    if (m_framesRendered == 0)
    {
        timeSinceLastTraditionalRender = 0.f;
        Adreno::ClockGetTime(&m_renderStart);
    }
    else
    {
        Adreno::ClockGetTime(&m_renderStop);
        timeSinceLastTraditionalRender = static_cast<float>(Adreno::Seconds(m_renderStop - m_renderStart));
        m_renderStart = m_renderStop;
    }

    //LOGI("timeSinceLastTraditionalRender=%f", timeSinceLastTraditionalRender);

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
    m_cameraPosition = Adreno::SplineEvaluate(m_splineT, &splinePoints[0], kSplinePointsNum);
    m_cameraPositionLookahead =
        Adreno::SplineEvaluate(Adreno::SplineTIncrement(m_splineT, .0625f, kSplinePointsNumFloat), &splinePoints[0], kSplinePointsNum);
    m_splineT = Adreno::SplineTIncrement(m_splineT, timeSinceLastTraditionalRender * .2f, kSplinePointsNumFloat);
    //LOGI("cameraPosition:(%f, %f, %f), cameraPositionLookahead:(%f, %f, %f)", \
        m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, m_cameraPositionLookahead.x, m_cameraPositionLookahead.y, m_cameraPositionLookahead.z);

}


//--------------------------------------------------------------------------------------
// Name: RenderSubScene()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::RenderSubScene()
{
    if(m_cameraStatus == CAMERA_MOVE){
        CameraUpdate();
    }else{
        Adreno::ClockGetTime(&m_renderStart);
    }

    //*** Clear the backbuffer and depth-buffer ***//
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    const glm::vec3 worldUp(0.f, 1.f, 0.f);
    const glm::mat4 localToWorld = glm::rotate(glm::mat4(), glm::radians(270.f), worldUp);

    m_prevViewProjection = m_cameraProjection * m_cameraView;

    //**** Update Camera ***//
    m_cameraView = glm::lookAt(m_cameraPosition, m_cameraPositionLookahead, worldUp);
    m_cameraProjection = glm::perspective(glm::radians(m_verticalFOV), 1.f / m_aspectRatio, m_cameraNear, m_cameraFar);

    //*** Apply jitter ***//
    if(m_mode != SGSR_OFF)
    {
        if (++m_jitterIndex >= (uint32_t) m_jitterPositions.size()) {
            m_jitterIndex = 0;
        }
        const glm::vec2 scaler{1.0f / float(m_sgsrRenderSize.x), -1.0f / float(m_sgsrRenderSize.y)};  //convert unit pixel space into projection offsets
        m_jitter = m_jitterPositions[m_jitterIndex] * scaler;

        m_cameraProjection[2][0] += m_jitter.x;
        m_cameraProjection[2][1] += m_jitter.y;
    }

    const glm::mat4 modelToClip =
        m_cameraProjection *
        m_cameraView *
        glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.f)) *
        localToWorld;

    glUniformMatrix4fv(m_modelToClipLoc, 1, GL_FALSE, &modelToClip[0][0]);
    glUniformMatrix4fv(m_modelRotationLocalLoc, 1, GL_FALSE, &localToWorld[0][0]);

    size_t indexBufferIndex = 0;
    const auto& adrenoModel = m_adrenoModelMerged;
    for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex) {
        // Draw the model
        const Adreno::Mesh &adrenoMesh = adrenoModel.Meshes[meshIndex];
        const Adreno::MeshSurfaceArray &adrenoMeshSurfaces = adrenoMesh.Surfaces;

        if (adrenoMeshSurfaces.NumSurfaces == 1) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
                          m_textureHandles[adrenoMeshSurfaces.SurfaceGet(0)->MaterialId]);
        }

        const Adreno::VertexFormat &adrenoVertexFormat = adrenoMesh.Vertices.Format;
        for (INT32 adrenoVertexFormatPropertiesIndex = 0;
             adrenoVertexFormatPropertiesIndex < adrenoVertexFormat.NumProperties;
             ++adrenoVertexFormatPropertiesIndex) {
            const Adreno::VertexProperty &vertexProperty = adrenoVertexFormat.Properties[adrenoVertexFormatPropertiesIndex];
            if (vertexProperty.Valid()) {
                const char *const vertexPropertyName = &vertexProperty.Usage.Name[0];
                const GLuint doNotHandle = UINT32_MAX;
                GLuint loc = doNotHandle;
                if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_POSITION)) {
                    loc = m_vertexLoc;
                } else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_NORMAL)) {
                    loc = m_normalLoc;
                } else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_TEXCOORD_0)) {
                    loc = m_textureLoc;
                }
                //don't handle other vertex attributes yet

                if (loc != doNotHandle) {
                    glVertexAttribPointer(
                        loc,
                        vertexProperty.NumValues(),
                        GL_FLOAT,
                        0,
                        adrenoVertexFormat.Stride,
                        &adrenoMesh.Vertices.Buffer[vertexProperty.Offset]);
                    glEnableVertexAttribArray(loc);
                }
                //LOGI( "%zu/%s:VertexPropertyType=%i,Offset=%u,Size=%u,NumValues=%u,IsNormalized=%i, loc=%i",
                //      adrenoVertexFormatPropertiesIndex, &vertexProperty.Usage.Name[0], vertexProperty.Type, vertexProperty.Offset, vertexProperty.Size(), vertexProperty.NumValues(), vertexProperty.IsNormalized(), loc);
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


//--------------------------------------------------------------------------------------
// Name: DispatchSGSR()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::DispatchSGSR()
{
    uint32_t inViewWidth = m_frameBuffer.m_nWidth;
    uint32_t inViewHeight = m_frameBuffer.m_nHeight;
    uint32_t outViewWidth = m_frameBuffer.m_nWidth * m_upscaleFactor + 0.5;
    uint32_t outViewHeight = m_frameBuffer.m_nHeight * m_upscaleFactor + 0.5;

    const auto curr_view_proj_matrix        = m_cameraProjection * m_cameraView;
    const auto inv_current_view_proj_matrix = glm::inverse( m_cameraView ) * glm::inverse( m_cameraProjection );
    const auto mt                           = m_prevViewProjection * inv_current_view_proj_matrix;

    float angleHor =
        glm::tan(glm::radians(m_verticalFOV / 2)) * inViewWidth / inViewHeight;

    bool bSameCamera = isCameraStill(curr_view_proj_matrix, m_prevViewProjection);

    float MinLerpContribution = 0.0;
    if (bSameCamera) {
        m_sameFrameNum += 1;
        if (m_sameFrameNum > 5) {
            MinLerpContribution = 0.3;
        }
        if(m_sameFrameNum == 0xFFFF){
            m_sameFrameNum = 1;
        }
    } else {
        m_sameFrameNum = 0;
    }

    if(m_mode == SGSR_TWO_PASS_FRAG){
        float fInViewWidth = inViewWidth;
        float fInViewHeight = inViewHeight;
        float fOutViewWidth = outViewWidth;
        float fOutViewHeight = outViewHeight;
        float BiasmaxViewportXScale = fOutViewWidth / fInViewWidth;
        float scaleFactor = glm::min(20.0, pow((fOutViewWidth / fInViewWidth) * (fOutViewHeight / fInViewHeight), 3.0));

        SGSR2_Frag::ResourceDescription rd2 = {"InputColor", reinterpret_cast<void *>(GL_RGBA16F), inViewWidth,inViewHeight};
        SGSR2_Frag::Resource colorTexture = {m_frameBuffer.m_hColorTexture, rd2};

        SGSR2_Frag::ResourceDescription rd3 = {"InputDepth", reinterpret_cast<void *>(GL_DEPTH24_STENCIL8),inViewWidth, inViewHeight};
        SGSR2_Frag::Resource depthTexture = {m_frameBuffer.m_hDepthTexture, rd3};

        SGSR2_Frag::ResourceDescription rd4 = {"InputVelocity", reinterpret_cast<void *>(GL_RGBA8), 1, 1};
        SGSR2_Frag::Resource velocityTexture = {m_velocity.m_hTextureHandle, rd4};

        SGSR2_Frag::Resource outputTexture = {};
        SGSR2_Frag::DispatchDescription dispatchDescription = {
            colorTexture,
            depthTexture,
            velocityTexture,
            outputTexture,
            {mt[0][0], mt[0][1], mt[0][2], mt[0][3],
             mt[1][0], mt[1][1], mt[1][2], mt[1][3],
             mt[2][0], mt[2][1], mt[2][2], mt[2][3],
             mt[3][0], mt[3][1], mt[3][2], mt[3][3]},
            {fInViewWidth, fInViewHeight},
            {fOutViewWidth, fOutViewHeight},
            {float(1.0 / fInViewWidth), float(1.0 / fInViewHeight)},
            {float(1.0 / fOutViewWidth), float(1.0 / fOutViewHeight)},
            {m_jitterPositions[m_jitterIndex].x, m_jitterPositions[m_jitterIndex].y},
            {BiasmaxViewportXScale, scaleFactor},
            angleHor,
            MinLerpContribution,
            m_isCameraCut ? float(1.0) : float(0.0),
            m_sameFrameNum,
        };
        m_sgsr2CoreFrag.ExecuteSGSR2(&dispatchDescription);
        m_outputColor.m_hTextureHandle = dispatchDescription.outputTexture.resource;

    }else {
        //pass color texture to InputOpaqueColor if no transparent object
        SGSR2_Comp::ResourceDescription rd1 = {"InputOpaqueColor", reinterpret_cast<void *>(GL_RGBA16F),
                                               inViewWidth, inViewHeight};
        SGSR2_Comp::Resource opaqueColorTexture = {m_frameBuffer.m_hColorTexture, rd1};

        SGSR2_Comp::ResourceDescription rd2 = {"InputColor", reinterpret_cast<void *>(GL_RGBA16F), inViewWidth,
                                               inViewHeight};
        SGSR2_Comp::Resource colorTexture = {m_frameBuffer.m_hColorTexture, rd2};

        SGSR2_Comp::ResourceDescription rd3 = {"InputDepth", reinterpret_cast<void *>(GL_DEPTH24_STENCIL8),
                                               inViewWidth, inViewHeight};
        SGSR2_Comp::Resource depthTexture = {m_frameBuffer.m_hDepthTexture, rd3};

        SGSR2_Comp::ResourceDescription rd4 = {"InputVelocity", reinterpret_cast<void *>(GL_RGBA8), 1, 1};
        SGSR2_Comp::Resource velocityTexture = {m_velocity.m_hTextureHandle, rd4};

        SGSR2_Comp::Resource outputTexture = {};
        uint8_t sgsr_mode = m_mode == SGSR_TWO_PASS ? TWO_PASS_MODE : THREE_PASS_MODE;
        SGSR2_Comp::DispatchDescription dispatchDescription = {
            opaqueColorTexture,
            colorTexture,
            depthTexture,
            velocityTexture,
            outputTexture,
            {inViewWidth, inViewHeight},
            {outViewWidth, outViewHeight},
            {float(1.0) / float(inViewWidth), float(1.0) / float(inViewHeight)},
            {float(1.0) / float(outViewWidth), float(1.0) / float(outViewHeight)},
            {m_jitterPositions[m_jitterIndex].x, m_jitterPositions[m_jitterIndex].y},
            {mt[0][0], mt[0][1], mt[0][2], mt[0][3],
             mt[1][0], mt[1][1], mt[1][2], mt[1][3],
             mt[2][0], mt[2][1], mt[2][2], mt[2][3],
             mt[3][0], mt[3][1], mt[3][2], mt[3][3]},
            1.0,
            angleHor,
            m_cameraNear,
            MinLerpContribution,
            m_sameFrameNum,
            m_isCameraCut ? uint32_t(1) : uint32_t(0),
            sgsr_mode
        };

        m_sgsr2CoreComp.ExecuteSGSR2(&dispatchDescription);

        m_outputColor.m_hTextureHandle = dispatchDescription.outputTexture.resource;
    }

    ADRENO_ASSERT(glGetError() == GL_NO_ERROR, __FILE__, __LINE__);
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
    m_frameBuffer.Begin();
    RenderSubScene();
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);

    //*** Dispatch SGSR2.0 ***//
    if(m_mode != SGSR_OFF) {
        DispatchSGSR();
        if(m_mode == SGSR_TWO_PASS_FRAG){  //sgsr2-2pass-fs mode will bind internal FBO, so need to bind external FBO back
            m_frameBuffer.Begin();
        }
        m_isCameraCut = false;
    }

    //avoid unnecessary GMEMStore'ing depth or stencil data, since only the color data from this buffer will be copied to the back buffer
    const size_t attachmentsToInvalidateNum = 2;
    const GLenum attachmentsToInvalidate[attachmentsToInvalidateNum] = { GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT };
    glInvalidateFramebuffer( GL_DRAW_FRAMEBUFFER, attachmentsToInvalidateNum, &attachmentsToInvalidate[0] );

    //*** Draw to backbuffer ***//
    glDisable(GL_DEPTH_TEST);
    m_frameBuffer.End();
    ClearFast(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, m_windowWidth, m_windowHeight);

    //*** Draw a quad that spans the entire screen ***//
    glUseProgram(m_compShaderProgram);
    uint32_t index = 0;
    glActiveTexture(GL_TEXTURE0 + index);
    if(m_mode != SGSR_OFF) {
        glBindTexture(GL_TEXTURE_2D, m_outputColor.m_hTextureHandle);
    }else{
        glBindTexture(GL_TEXTURE_2D, m_frameBuffer.GetColorTexture());
    }
    glDrawArrays( GL_TRIANGLES, 0, 3 );
    glEnable(GL_DEPTH_TEST);
    ++m_framesRendered;
    OverlayTextDraw();
}
