//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef SCENE_H
#define SCENE_H

//--------------------------------------------------------------------------------------
// Name: g_internalFormat, g_format, g_type
// Desc: Specifies the format of the MSAA framebuffers
//--------------------------------------------------------------------------------------
GLenum g_format			= GL_RGB;
GLint g_internalFormat	= GL_RGB8;
GLenum g_type			= GL_UNSIGNED_BYTE;

// Wrapper for FBO objects and properties
struct FrameBufferObject
{
    UINT32 m_nWidth;
    UINT32 m_nHeight;

    UINT32 m_hFrameBuffer;
    UINT32 m_hRenderBuffer;
    UINT32 m_hColorBuffer;
};

//--------------------------------------------------------------------------------------
// Name: class CSample
// Desc: The main application class for this sample
//--------------------------------------------------------------------------------------
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
    VOID RenderScene();
    VOID RenderScreenAlignedQuad(UINT32);

    VOID DestroyFBO(FrameBufferObject*);
    BOOL CreateFBO(UINT32,UINT32,UINT32,UINT32,UINT32,FrameBufferObject**,BOOL);

    BOOL CreateIntermediateBlitFBO(UINT32,UINT32,UINT32,UINT32,UINT32,FrameBufferObject**);

    GLuint m_renderSceneProgram;
    GLuint m_fullScreenPassProgram;
    GLint m_fullScreenPassTextureUniform;

    FrameBufferObject* m_pOffscreenFBOStandard;
    FrameBufferObject* m_pOffscreenFBOOptimized;
    FrameBufferObject* m_pIntermediateBlitFBO;

    FLOAT  m_aspectRatio;

    CFrmFontGLES           m_Font;
    BOOL                   m_MSAAOptimizationEnabled;
    
    PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC m_glFramebufferTexture2DMultisampleEXT;
};

#endif // SCENE_H
