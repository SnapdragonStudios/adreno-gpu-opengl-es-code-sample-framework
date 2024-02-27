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
typedef void (GL_APIENTRYP PFNGLQCOMTEXESTIMATEMOTION) (GLuint ref, GLuint target, GLuint output);
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

    GLuint                              m_offscreenFramebufferTextureColor;
    GLuint                              m_offscreenFramebufferTextureDepth;
    GLuint                              m_offscreenFramebuffer;

    ResourceHandleArray<2>              m_luminanceTextures;
    ResourceHandleArray<2>              m_luminanceFramebuffers;
    
    GLuint                              m_motionVectorsTexture;
    GLuint                              m_motionVectorsFramebuffer;
    PFNGLQCOMTEXESTIMATEMOTION          m_glTexEstimateMotionQCOM;

    BOOL InitShaders();

    const CHAR*                         m_windowTitle;
    UINT32                              m_windowWidth;
    UINT32                              m_windowHeight;
    GLint                               m_motionEstimationSearchBlockX;
    GLint                               m_motionEstimationSearchBlockY;
    EGLint                              m_offscreenRenderTexturesWidth;
    EGLint                              m_offscreenRenderTexturesHeight;
    FLOAT                               m_aspectRatio;
    
    GLuint                              m_mainShader;
    const CHAR*                         m_mainVertexShader;    
    const CHAR*                         m_mainFragmentShader;    
    
    GLuint                              m_fullscreenCopyShader;
    const CHAR*                         m_fullscreenTriangleVertexShader;
    const CHAR*                         m_fullscreenCopyFragmentShader;    

    GLuint                              m_rgbaToLuminanceShader;
    const CHAR*                         m_rgbaToLuminanceFragmentShader;

    GLuint                              m_vertexLoc;
    GLuint                              m_normalLoc;
    GLuint                              m_textureLoc;
    GLuint                              m_modelToClipLoc;
    GLuint                              m_modelRotationLocalLoc;

    size_t                              m_framesRendered = 0;
    enum Mode                           {MODE_FIRST = 0, MODE_MOTION_VECTORS = MODE_FIRST, MODE_REGULAR, MODE_NUM} m_mode = MODE_FIRST;

    timespec                            m_renderStart, m_renderStop;
    const size_t                        kSplinePointsNum = 4;
    float                               m_splineT = static_cast<float>(kSplinePointsNum - 1);

    VOID RenderTraditional(const float targetRenderTime);
    inline GLint MotionVectorsTexturePixels(const GLint luminancePixelsNum, const GLint motionEstimationSearchBlockPixelsNum) const
    {
        ADRENO_ASSERT(luminancePixelsNum > 0, __FILE__, __LINE__);
        ADRENO_ASSERT(motionEstimationSearchBlockPixelsNum > 0, __FILE__, __LINE__);
        ADRENO_ASSERT(luminancePixelsNum % motionEstimationSearchBlockPixelsNum == 0, __FILE__, __LINE__);
        
        return luminancePixelsNum/motionEstimationSearchBlockPixelsNum;
    }
    inline GLint MotionVectorsTextureWidthPixels() const
    {
        return MotionVectorsTexturePixels(m_offscreenRenderTexturesWidth, m_motionEstimationSearchBlockX);
    }
    inline GLint MotionVectorsTextureHeightPixels() const
    {
        return MotionVectorsTexturePixels(m_offscreenRenderTexturesHeight, m_motionEstimationSearchBlockY);
    }

    VOID OverlayTextDraw(const float timeSinceLastRender);
    VOID DebugLuminanceAndMotionVectorsBuffersWriteToTGA();
};

#endif // SCENE_H
