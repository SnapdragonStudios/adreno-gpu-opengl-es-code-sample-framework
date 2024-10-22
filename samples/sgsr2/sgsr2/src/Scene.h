// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

#include "FrmApplication.h"
#include "FrmModelIO.h"
#include "FrmResourceGLES.h"
#include "FrmStdLib.h"
#include <array>
#include "FrmFontGLES.h"
#include <tinygltf/examples/common/glm/glm/glm.hpp>
#include "api/gles/sgsr2_core_cs.h"
#include "api/gles/sgsr2_core_fs.h"

#define CAMERA_MOVE             0
#define CAMERA_STILL            1

typedef enum ALGORITHM_MODE
{
    SGSR_THREE_PASS,
    SGSR_TWO_PASS,
    SGSR_TWO_PASS_FRAG,
    SGSR_OFF
}ALGORITHM_MODE;

//--------------------------------------------------------------------------------------
// Name: class CSample
// Desc: The main application class for this sample
//--------------------------------------------------------------------------------------
typedef void (GL_APIENTRYP PFNGLQCOMFRAMEEXTRAPOLATION) (GLuint src1, GLuint src2, GLuint output, float scaleFactor);


//--------------------------------------------------------------------------------------
// Name: class CFrmMainFrameBufferObject
// Desc: Wrapper class for rendering to offscreen textures via FBOs
//--------------------------------------------------------------------------------------
class CFrmMainFrameBufferObject
{
public:
    CFrmMainFrameBufferObject();
    ~CFrmMainFrameBufferObject();

    BOOL   Create( UINT32 nWidth, UINT32 nHeight);
    VOID   Destroy();
    VOID   Begin();
    VOID   End();
    UINT32 GetColorTexture() { return m_hColorTexture; }
    UINT32 GetDepthTexture() { return m_hDepthTexture; }

public:
    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nFormat;
    UINT32 m_nType;

    UINT32 m_hColorTexture;
    UINT32 m_hDepthTexture;
    UINT32 m_hFrameBuffer;

    struct
    {
        INT32 x, y, w, h;
    } m_PreviousViewport;
};




class CSample : public CFrmApplication
{

public:
    virtual BOOL Initialize();
    virtual BOOL Resize();
    virtual VOID Destroy();
    virtual VOID Update();
    virtual VOID Render();

    CSample( const CHAR* strName );

private:
    glm::uvec2                          m_sgsrRenderSize;
    CFrmMainFrameBufferObject           m_frameBuffer;
    const CHAR*                         m_compVertexSource;
    const CHAR*                         m_compFragSource;
    GLuint                              m_compShaderProgram;
    GLuint                              m_vertexPos;
    GLuint                              m_vertexTex;

    SGSR2_Comp::SGSR2_Core              m_sgsr2CoreComp;
    SGSR2_Frag::SGSR2_Core              m_sgsr2CoreFrag;
    float                               m_upscaleFactor;
    CFrmTexture                         m_velocity;
    CFrmTexture                         m_outputColor;
    glm::mat4                           m_prevViewProjection;
    glm::vec2                           m_jitter;
    uint32_t                            m_jitterIndex;
    std::array<glm::vec2, 32>           m_jitterPositions;
    CFrmFontGLES                        m_font;
    uint32_t                            m_sameFrameNum = 1;
    ALGORITHM_MODE                      m_mode = SGSR_OFF;
    bool                                m_isCameraCut = true;

    glm::mat4                           m_cameraView;
    glm::mat4                           m_cameraProjection;
    float                               m_verticalFOV;
    float                               m_cameraNear;
    float                               m_cameraFar;
    glm::vec3                           m_cameraPosition;
    glm::vec3                           m_cameraPositionLookahead;
    uint8_t                             m_cameraStatus = CAMERA_MOVE;

    VOID                                PrepareSGSR();
    VOID                                OverlayTextDraw();
    VOID                                RenderSubScene();
    VOID                                DispatchSGSR();
    VOID                                CameraUpdate();

    Adreno::Model                       m_adrenoModelMerged;
    std::vector<std::string>            m_textureFilepaths;
    Adreno::MeshesSortedByMaterialId    m_meshesSortedByMaterialId;
    ResourceHandleArray<1024>           m_indexBufferHandles;
    ResourceHandleArray<1024>           m_textureHandles;
    UINT32                              m_baseColorDefault_MaterialId;

    BOOL InitShaders();

    const CHAR*                         m_windowTitle;
    UINT32                              m_windowWidth;
    UINT32                              m_windowHeight;
    FLOAT                               m_aspectRatio;
    GLuint                              m_shaderProgram;
    GLuint                              m_vertexLoc;
    GLuint                              m_normalLoc;
    GLuint                              m_textureLoc;
    GLuint                              m_modelToClipLoc;
    GLuint                              m_modelRotationLocalLoc;
    const CHAR*                         m_fragmentShaderProgram;
    const CHAR*                         m_vertexShaderProgram;

    timespec                            m_renderStart, m_renderStop;
    size_t                              m_framesRendered = 0;
    const size_t                        kSplinePointsNum = 4;
    float                               m_splineT = static_cast<float>(kSplinePointsNum - 1);
};

#endif // SCENE_H
