//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include <FrmPlatform.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <FrmApplication.h>
#include <OpenGLES/FrmShader.h>
#include <OpenGLES/FrmFontGLES.h>
#include <OpenGLES/FrmPackedResourceGLES.h>
#include <OpenGLES/FrmUserInterfaceGLES.h>
#include <OpenGLES/FrmUtilsGLES.h>
#include "Scene.h"

#define VERTEX_ATTR_LOC 0
#define COLOR_ATTR_LOC 1
#define UV_ATTR_LOC 1

//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    return new CSample( "MSAA" );
}

//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample( const CHAR* strName ) : CFrmApplication( strName )
{
    m_aspectRatio   = (FLOAT)m_nWidth / (FLOAT)m_nHeight;

    m_renderSceneProgram = 0;
    m_fullScreenPassProgram = 0;
    m_fullScreenPassTextureUniform = 0;

    m_pOffscreenFBOOptimized = NULL;
    m_pOffscreenFBOStandard = NULL;

    m_MSAAOptimizationEnabled = FALSE;
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc:
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
    const char*const GL_EXT_multisampled_render_to_texture_cStr = "GL_EXT_multisampled_render_to_texture";
    const bool GL_EXT_multisampled_render_to_texture_supported = FrmGLExtensionSupported(GL_EXT_multisampled_render_to_texture_cStr);
    if(!GL_EXT_multisampled_render_to_texture_supported)
    {
        LOGI("Extension %s ia not supported; sample can't run", GL_EXT_multisampled_render_to_texture_cStr);
        ADRENO_ASSERT(GL_EXT_multisampled_render_to_texture_supported, __FILE__, __LINE__);
        return FALSE;//sample can't run
    }    
    m_glFramebufferTexture2DMultisampleEXT = (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)eglGetProcAddress("glFramebufferTexture2DMultisampleEXT");
    ADRENO_ASSERT(m_glFramebufferTexture2DMultisampleEXT, __FILE__, __LINE__);
    
    // Setup font to be used for UI
    {
        if (FALSE == m_Font.Create("Tuffy16.pak"))
            return FALSE;
        m_Font.SetScaleFactors(3.0f, 3.0f);
    }

    // Create program that renders the scene
    {
        FRM_SHADER_ATTRIBUTE attrs[2];
        attrs[0].strName = "inVertex";
        attrs[0].nLocation = VERTEX_ATTR_LOC;
        attrs[1].strName = "inColor";
        attrs[1].nLocation = COLOR_ATTR_LOC;

        FrmCompileShaderProgramFromFile("pass_through_color.vs", "vertex_color.fs",
                                        &m_renderSceneProgram, &attrs[0], 2);
    }

    // Create program that mimics a post processing pipeline. In this case we are using a simple vignetting shader to exemplify this.
    {
        FRM_SHADER_ATTRIBUTE attrs[2];
        attrs[0].strName = "inVertex";
        attrs[0].nLocation = VERTEX_ATTR_LOC;
        attrs[1].strName = "inUV";
        attrs[1].nLocation = UV_ATTR_LOC;

        FrmCompileShaderProgramFromFile("pass_through_uv.vs", "vignetting_effect.fs",
                                        &m_fullScreenPassProgram, &attrs[0], 2);

        m_fullScreenPassTextureUniform = glGetUniformLocation(m_fullScreenPassProgram, "g_Texture");
    }

    // Create an offscreen multi-sample FBO to render to in the 'standard' way
    if( FALSE == CreateFBO( m_nWidth, m_nHeight, g_format, g_type, g_internalFormat, &m_pOffscreenFBOStandard, FALSE ) )
        return FALSE;

    // Create an offscreen multi-sample FBO to render to with optimization extension
    if( FALSE == CreateFBO( m_nWidth, m_nHeight, g_format, g_type, g_internalFormat, &m_pOffscreenFBOOptimized, TRUE ) )
        return FALSE;

    // To apply a post effect pipeline the 'standard' way requires a blit to resolve the MSAA target to a single sample intermediate FBO
    if( FALSE == CreateIntermediateBlitFBO( m_nWidth, m_nHeight, g_format, g_type, g_internalFormat, &m_pIntermediateBlitFBO ) )
        return FALSE;

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: CreateFBO()
// Desc: Create objects needed for rendering to an FBO with 4xMSAA
//--------------------------------------------------------------------------------------
BOOL CSample::CreateFBO( UINT32 width, UINT32 height, UINT32 format, UINT32 type,
                         UINT32 internalFormat, FrameBufferObject** ppFBO, BOOL isOptimized)
{
    (*ppFBO) = new FrameBufferObject;
    (*ppFBO)->m_nWidth  = width;
    (*ppFBO)->m_nHeight = height;

    // Create the color buffer
    if(isOptimized)
    {
        // OPTIMIZED
        glGenTextures(1, &(*ppFBO)->m_hColorBuffer);
        glBindTexture(GL_TEXTURE_2D, (*ppFBO)->m_hColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else
    {
        // NOT OPTIMIZED
        glGenRenderbuffers(1, &(*ppFBO)->m_hColorBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, (*ppFBO)->m_hColorBuffer);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, internalFormat, width, height);
    }

    // Create the multi-sample depth buffer
    glGenRenderbuffers( 1, &(*ppFBO)->m_hRenderBuffer );
    glBindRenderbuffer( GL_RENDERBUFFER, (*ppFBO)->m_hRenderBuffer );
    glRenderbufferStorageMultisample( GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT24, width, height );

    glGenFramebuffers(1, &(*ppFBO)->m_hFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, (*ppFBO)->m_hFrameBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, (*ppFBO)->m_hRenderBuffer);

    if(isOptimized) {
        m_glFramebufferTexture2DMultisampleEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (*ppFBO)->m_hColorBuffer, 0, 4);
    } else {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, (*ppFBO)->m_hColorBuffer);
    }

    GLenum fbStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( GL_FRAMEBUFFER_COMPLETE != fbStatus ) {
        FrmLogMessage("Framebuffer could not be created properly.");
        return FALSE;
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: CreateFBO()
// Desc: To apply a post effect pipeline the 'standard' way requires a blit to resolve the MSAA target to a single sample intermediate FBO
//--------------------------------------------------------------------------------------
BOOL CSample::CreateIntermediateBlitFBO( UINT32 width, UINT32 height, UINT32 format, UINT32 type,
                         UINT32 internalFormat, FrameBufferObject** ppFBO)
{
    (*ppFBO) = new FrameBufferObject;
    (*ppFBO)->m_nWidth  = width;
    (*ppFBO)->m_nHeight = height;

    glGenTextures(1, &(*ppFBO)->m_hColorBuffer);
    glBindTexture(GL_TEXTURE_2D, (*ppFBO)->m_hColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &(*ppFBO)->m_hFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, (*ppFBO)->m_hFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,  (*ppFBO)->m_hColorBuffer, 0);

    GLenum fbStatus = glCheckFramebufferStatus( GL_FRAMEBUFFER );
    if( GL_FRAMEBUFFER_COMPLETE != fbStatus ) {
        FrmLogMessage("Framebuffer could not be created properly.");
        return FALSE;
    }

    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glBindRenderbuffer( GL_RENDERBUFFER, 0 );
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: DestroyFBO()
// Desc: Destroy the FBO
//--------------------------------------------------------------------------------------
VOID CSample::DestroyFBO( FrameBufferObject* pFBO )
{
    glDeleteFramebuffers( 1, &pFBO->m_hFrameBuffer );
    //glDeleteRenderbuffers( 1, &pFBO->m_hRenderBuffer );
    glDeleteRenderbuffers( 1, &pFBO->m_hColorBuffer );
    delete pFBO;
}

//--------------------------------------------------------------------------------------
// Name: Resize()
// Desc:
//--------------------------------------------------------------------------------------
BOOL CSample::Resize()
{
    glViewport( 0, 0, m_nWidth, m_nHeight );
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::Destroy()
{
    glDeleteProgram( m_renderSceneProgram );
    glDeleteProgram(m_fullScreenPassProgram);

    DestroyFBO( m_pOffscreenFBOStandard );
    DestroyFBO( m_pOffscreenFBOOptimized );
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
    m_Input.GetPointerState( &pntState, &newMousePose );

    // Toggle optimization if user touches the screen anywhere.
    if (pntState & FRM_INPUT::POINTER_RELEASED)
    {
        m_MSAAOptimizationEnabled = !m_MSAAOptimizationEnabled;
    }
}

//--------------------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Render the scene. For demo purposes just drawing one triangle will suffice
//--------------------------------------------------------------------------------------
VOID CSample::RenderScene()
{
    FLOAT fSize = 0.5f;
    FLOAT VertexPositions[] =
            {
                    0.0f,  +fSize*m_aspectRatio, 0.0f, 1.0f,
                    -fSize, -fSize*m_aspectRatio, 0.0f, 1.0f,
                    +fSize, -fSize*m_aspectRatio, 0.0f, 1.0f,
            };

    FLOAT VertexColors[] = {1.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 1.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f, 1.0f
    };

    // Clear the back buffer and depth-buffer
    glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set the shader program and the texture
    glUseProgram( m_renderSceneProgram );

    // Draw the colored triangle
    glVertexAttribPointer( VERTEX_ATTR_LOC, 4, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( VERTEX_ATTR_LOC );

    glVertexAttribPointer( COLOR_ATTR_LOC, 4, GL_FLOAT, 0, 0, VertexColors);
    glEnableVertexAttribArray( COLOR_ATTR_LOC );

    glDrawArrays( GL_TRIANGLE_STRIP, 0, 3 );

    glDisableVertexAttribArray( VERTEX_ATTR_LOC );
    glDisableVertexAttribArray( COLOR_ATTR_LOC );

    CHAR fpsString[40];
    _FRMCOLOR textColor = FRMCOLOR_GREEN;

    if (m_MSAAOptimizationEnabled) {
        FrmSprintf(fpsString, 40, "Optimized\nPress anywhere on the screen\nto toggle optimization");
    } else {
        FrmSprintf(fpsString, 40, "Not Optimized\nPress anywhere on the screen\nto toggle optimization");
        textColor = FRMCOLOR_RED;
    }

    m_Font.DrawText(m_nWidth/2.0f, 100, textColor, fpsString, FRM_FONT_CENTER);
}

//--------------------------------------------------------------------------------------
// Name: Render()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::Render()
{
    if (m_MSAAOptimizationEnabled) {
        // Render the scene into the offscreen multi-sample FBO
        glBindFramebuffer( GL_FRAMEBUFFER, m_pOffscreenFBOOptimized->m_hFrameBuffer );
        RenderScene();

        // Invalidate the depth target as it will not be used anymore. This is important as it will hint the GPU to
        // not perform 'GMEM Store' operations on the depth target.
        // For more information on this, look at the GMEM Store Best Practice Sample
        GLenum invalidate[1] = { GL_DEPTH_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalidate[0]);

        // Setup the back buffer and render our post effects output to it.
        glBindFramebuffer( GL_FRAMEBUFFER, 0);
        glViewport(0.0f, 0.0f, m_nWidth, m_nHeight);

        // Note: Even though RenderScreenAlignedQuad will draw all pixels, it is important in mobile tiling architectures
        // to clear the FBO, this will instruct the driver not to load its previous content into GMEM.
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        RenderScreenAlignedQuad(m_pOffscreenFBOOptimized->m_hColorBuffer);
    }
    else {
        // Render the scene into the offscreen multi-sample FBO
        glBindFramebuffer( GL_FRAMEBUFFER, m_pOffscreenFBOStandard->m_hFrameBuffer );
        RenderScene();

        // Invalidate the depth target as it will not be used anymore. This is important as it will hint the GPU to
        // not perform 'GMEM Store' operations on the depth target.
        // For more information on this, look at the GMEM Store Best Practice Sample
        GLenum invalidate[1] = { GL_DEPTH_ATTACHMENT };
        glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &invalidate[0]);

        // Bind the offscreen multisample FBO to the read framebuffer
        glBindFramebuffer( GL_READ_FRAMEBUFFER, m_pOffscreenFBOStandard->m_hFrameBuffer );

        // Bind the default framebuffer as the draw framebuffer
        glBindFramebuffer( GL_DRAW_FRAMEBUFFER, m_pIntermediateBlitFBO->m_hFrameBuffer );

        // Blit the multisampled fb into the main fb
        //	IMPORTANT:	- When blitting from a multisample framebuffer, the specified
        //				source and destination rectangles MUST be the same size.
        //				- Also, the destination framebuffer (the GL_DRAW_FRAMEBUFFER)
        //				must not be a multisample framebuffer.
        // This blit will resolve the multisampled framebuffer
        glBlitFramebuffer( 0, 0, m_pOffscreenFBOStandard->m_nWidth, m_pOffscreenFBOStandard->m_nHeight,
                           0, 0, m_pOffscreenFBOStandard->m_nWidth, m_pOffscreenFBOStandard->m_nHeight,
                           GL_COLOR_BUFFER_BIT, GL_NEAREST );	// Color only, and filtering

        // Nw setup the back buffer and render our post effects output to it.
        glBindFramebuffer( GL_FRAMEBUFFER, 0);
        glViewport(0.0f, 0.0f, m_nWidth, m_nHeight);

        // Note: Even though RenderScreenAlignedQuad will draw all pixels, it is important in mobile tiling architectures
        // to clear the FBO, this will instruct the driver not to load its previous content into GMEM.
        glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );
        glClear( GL_COLOR_BUFFER_BIT );
        RenderScreenAlignedQuad(m_pIntermediateBlitFBO->m_hColorBuffer);
    }
}

//--------------------------------------------------------------------------------------
// Name: RenderScreenAlignedQuad
// Desc: Draws a full-screen rectangle
//--------------------------------------------------------------------------------------
VOID CSample::RenderScreenAlignedQuad(UINT32 textureHandle)
{
    glDisable( GL_DEPTH_TEST );

    const FLOAT32 VertexPositions[] =
            {
                    +1.0, +1.0f, 0.0f, 1.0f,
                    -1.0, +1.0f, 0.0f, 1.0f,
                    +1.0, -1.0f, 0.0f, 1.0f,
                    -1.0, -1.0f, 0.0f, 1.0f,
            };

    const FLOAT32 VertexUVs[] =
            {
                    1.0f, 1.0f,
                    0.0f, 1.0f,
                    1.0f, 0.0f,
                    0.0f, 0.0f,
            };

    // Set the shader program and the texture
    glUseProgram( m_fullScreenPassProgram );

    // Draw the colored triangle
    glVertexAttribPointer( VERTEX_ATTR_LOC, 4, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( VERTEX_ATTR_LOC );

    glVertexAttribPointer( UV_ATTR_LOC, 2, GL_FLOAT, 0, 0, VertexUVs);
    glEnableVertexAttribArray( UV_ATTR_LOC );

    // Set texture to draw full screen pass
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glUniform1i(m_fullScreenPassTextureUniform, 0);

    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

    glDisableVertexAttribArray( VERTEX_ATTR_LOC );
    glDisableVertexAttribArray( UV_ATTR_LOC );

    glEnable( GL_DEPTH_TEST );
}
