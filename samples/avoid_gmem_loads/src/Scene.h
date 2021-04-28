// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

struct FrameBufferObject
{
    UINT32 m_nWidth;
    UINT32 m_nHeight;

    UINT32 m_hFrameBuffer;
    UINT32 m_hDepthTexture;
    UINT32 m_hTexture;
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

    BOOL InitShaders();
    BOOL CreateFBO( UINT32 nWidth, UINT32 nHeight, UINT32 nFormat, UINT32 nType, UINT32 nInternalFormat, FrameBufferObject** ppFBO );
    VOID DestroyFBO(FrameBufferObject*);
    VOID BeginFBO(FrameBufferObject*);
    VOID EndFBO(FrameBufferObject*);
    VOID RenderScene(FLOAT);

    const CHAR*  g_strWindowTitle;
    const CHAR* g_strOverlayFragmentShader;
    const CHAR* g_strOverlayVertexShader;
    const CHAR* g_strFragmentShader;
    const CHAR* g_strVertexShader;
    UINT32 g_nWindowWidth;
    UINT32 g_nWindowHeight;
    FLOAT  g_fAspectRatio;

    FrameBufferObject* g_pOffscreenFBO0;
    FrameBufferObject* g_pOffscreenFBO1;

    GLuint       g_hShaderProgram        ;
    GLuint       g_hOverlayShaderProgram ;
    GLuint       g_hModelViewMatrixLoc   ;
    GLuint       g_hProjMatrixLoc        ;
    GLuint       g_hVertexLoc            ;
    GLuint       g_hColorLoc             ;

    CFrmFontGLES           m_Font;
    CFrmUserInterfaceGLES  m_UserInterface;
    BOOL                   m_bClearAll;

};

#endif // SCENE_H
