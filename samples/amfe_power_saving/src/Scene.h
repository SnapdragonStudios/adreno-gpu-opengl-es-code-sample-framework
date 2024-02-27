// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

#include "FrmApplication.h"
#include "FrmFontGLES.h"
#include "FrmModelIO.h"
#include "FrmResourceGLES.h"
#include "FrmStdLib.h"


//--------------------------------------------------------------------------------------
// Name: class CSample
// Desc: The main application class for this sample
//--------------------------------------------------------------------------------------
typedef void (GL_APIENTRYP PFNGLQCOMFRAMEEXTRAPOLATION) (GLuint src1, GLuint src2, GLuint output, float scaleFactor);
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
    Adreno::Model                       m_adrenoModelMerged;
    std::vector<std::string>            m_textureFilepaths;
    Adreno::MeshesSortedByMaterialId    m_meshesSortedByMaterialId;
    ResourceHandleArray<1024>           m_indexBufferHandles;
    ResourceHandleArray<1024>           m_textureHandles;
    UINT32                              m_baseColorDefault_MaterialId;
    CFrmFontGLES                        m_font;

    GLint                               m_backbufferTextureType;
    
    ResourceHandleArray<2>              m_offscreenFramebufferTexturesColor;
    ResourceHandleArray<2>              m_offscreenFramebufferTexturesDepth;
    ResourceHandleArray<2>              m_offscreenFramebuffers;
    
    GLuint                              m_extrapolatedTexture;
    GLuint                              m_extrapolatedFramebuffer;
    PFNGLQCOMFRAMEEXTRAPOLATION         m_glExtrapolateTex2DQCOM;

    BOOL InitShaders();

    const CHAR*                         m_windowTitle;
    UINT32                              m_windowWidth;
    UINT32                              m_windowHeight;
    EGLint                              m_eglSurfaceWidth;
    EGLint                              m_eglSurfaceHeight;
    FLOAT                               m_aspectRatio;
    
    GLuint                              m_mainShader;
    const CHAR*                         m_mainVertexShader;    
    const CHAR*                         m_mainFragmentShader;    
    
    GLuint                              m_fullscreenCopyShader;
    const CHAR*                         m_fullscreenTriangleVertexShader;        
    const CHAR*                         m_fullscreenCopyFragmentShader;    
    
    GLuint                              m_vertexLoc;
    GLuint                              m_normalLoc;
    GLuint                              m_textureLoc;
    GLuint                              m_modelToClipLoc;
    GLuint                              m_modelRotationLocalLoc;

    size_t                              m_framesTraditionalRendered = 0;
    bool                                m_extrapolateThisFrame = false;
    bool                                m_extrapolatingEveryOtherFrame = true;

    timespec                            m_traditionalRenderStart, m_traditionalRenderStop;
    const size_t                        kSplinePointsNum = 4;
    float                               m_splineT = static_cast<float>(kSplinePointsNum - 1);

    VOID RenderTraditional(const float targetRenderTime);
    VOID OverlayTextDraw();
    VOID DebugExtrapolateFrameWriteToTGA();
};

#endif // SCENE_H
