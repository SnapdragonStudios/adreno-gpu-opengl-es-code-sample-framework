// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

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

#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif

//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    return new CSample( "RenderToTexture" );
}

//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample( const CHAR* strName ) : CFrmApplication( strName )
{
    g_strWindowTitle = strName;
    g_nWindowWidth   = 1080;
    g_nWindowHeight  = 2160;
    g_fAspectRatio   = (FLOAT)g_nWindowWidth / (FLOAT)g_nWindowHeight;


    g_pOffscreenFBO0 = NULL;
    g_pOffscreenFBO1 = NULL;

    g_hShaderProgram        = 0;
    g_hOverlayShaderProgram = 0;
    g_hModelViewMatrixLoc   = 0;
    g_hProjMatrixLoc        = 0;
    g_hVertexLoc            = 0;
    g_hColorLoc             = 1;

    m_bClearAll = FALSE;
}

//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    g_strVertexShader =
            "uniform   mat4 g_matModelView;                                      \n"
            "uniform   mat4 g_matProj;                                           \n"
            "                                                                    \n"
            "attribute vec4 g_vPosition;	 								     \n"
            "attribute vec3 g_vColor; 			   		 		                 \n"
            "                                                                    \n"
            "varying   vec3 g_vVSColor;                                          \n"
            "																	 \n"
            "void main()														 \n"
            "{																	 \n"
            "    vec4 vPositionES = g_matModelView * g_vPosition;                \n"
            "    gl_Position  = g_matProj * vPositionES;                         \n"
            "    g_vVSColor = g_vColor;                                          \n"
            "}																   	 \n";

    g_strFragmentShader =
            "#ifdef GL_FRAGMENT_PRECISION_HIGH									 \n"
            "   precision highp float;											 \n"
            "#else																 \n"
            "   precision mediump float;										 \n"
            "#endif																 \n"
            "																	 \n"
            "varying   vec3      g_vVSColor;                                     \n"
            "																	 \n"
            "void main()														 \n"
            "{																	 \n"
            "    gl_FragColor = vec4( g_vVSColor, 1.0 );                         \n"
            "}																	 \n";


//--------------------------------------------------------------------------------------
// Name: g_strOverlayVertexShader / g_strOverlayFragmentShader
// Desc: The vertex and fragment shader programs
//--------------------------------------------------------------------------------------
    g_strOverlayVertexShader =
            "attribute vec4 g_vVertex;                               \n"
            "varying   vec2 g_vTexCoord;                             \n"
            "														 \n"
            "void main()											 \n"
            "{														 \n"
            "    gl_Position.x = g_vVertex.x;                        \n"
            "    gl_Position.y = g_vVertex.y;                        \n"
            "    gl_Position.z = 0.0;                                \n"
            "    gl_Position.w = 1.0;                                \n"
            "                                                        \n"
            "    g_vTexCoord.x = g_vVertex.z;                        \n"
            "    g_vTexCoord.y = g_vVertex.w;                        \n"
            "}                                                       \n";

    g_strOverlayFragmentShader =
            "#ifdef GL_FRAGMENT_PRECISION_HIGH						 \n"
            "   precision highp float;					             \n"
            "#else													 \n"
            "   precision mediump float;					         \n"
            "#endif							                         \n"
            "uniform sampler2D g_Texture;                            \n"
            "varying vec2      g_vTexCoord;                          \n"
            "                                                        \n"
            "void main()                                             \n"
            "{                                                       \n"
            "    gl_FragColor = texture2D( g_Texture, g_vTexCoord ); \n"
            "}                                                       \n";

    return TRUE;
}



//--------------------------------------------------------------------------------------
// Name: CreateFBO()
// Desc: Create objects needed for rendering to an FBO
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// Name: CreateFBO()
// Desc:
//--------------------------------------------------------------------------------------
BOOL CSample::CreateFBO( UINT32 nWidth, UINT32 nHeight, UINT32 nFormat, UINT32 nType,
                         UINT32 nInternalFormat, FrameBufferObject** ppFBO )
{
    (*ppFBO) = new FrameBufferObject;
    (*ppFBO)->m_nWidth  = nWidth;
    (*ppFBO)->m_nHeight = nHeight;

    glGenFramebuffers( 1, &(*ppFBO)->m_hFrameBuffer );
    glBindFramebuffer( GL_FRAMEBUFFER, (*ppFBO)->m_hFrameBuffer );

    // Create an offscreen texture
    glGenTextures( 1, &(*ppFBO)->m_hTexture );
    glBindTexture( GL_TEXTURE_2D, (*ppFBO)->m_hTexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE );
    glTexImage2D( GL_TEXTURE_2D, 0, nInternalFormat, nWidth, nHeight, 0, nFormat, nType, NULL );
    glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, (*ppFBO)->m_hTexture, 0 );

    glGenTextures(1, &(*ppFBO)->m_hDepthTexture);
    glBindTexture(GL_TEXTURE_2D, (*ppFBO)->m_hDepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, nWidth, nHeight, 0, GL_DEPTH_STENCIL,
                     GL_UNSIGNED_INT_24_8, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                               (*ppFBO)->m_hDepthTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_TEXTURE_2D,
                               (*ppFBO)->m_hDepthTexture, 0);

    if( GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus( GL_FRAMEBUFFER ) )
        return FALSE;

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
    glDeleteTextures( 1, &pFBO->m_hTexture );
    glDeleteFramebuffers( 1, &pFBO->m_hFrameBuffer );
    glDeleteTextures(1, &pFBO->m_hDepthTexture);
    delete pFBO;
}


//--------------------------------------------------------------------------------------
// Name: BeginFBO()
// Desc: Bind objects for rendering to the frame buffer object
//--------------------------------------------------------------------------------------
VOID CSample::BeginFBO( FrameBufferObject* pFBO)
{
    glBindFramebuffer( GL_FRAMEBUFFER, pFBO->m_hFrameBuffer );
    glViewport( 0, 0, 4096, 4096 );
}


//--------------------------------------------------------------------------------------
// Name: EndFBO()
// Desc: Restore rendering back to the primary frame buffer
//--------------------------------------------------------------------------------------
VOID CSample::EndFBO( FrameBufferObject* pFBO )
{
    glBindFramebuffer( GL_FRAMEBUFFER, 0 );
    glViewport( 0, 0, g_nWindowWidth, g_nWindowHeight );
}



//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc:
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
    if( FALSE == m_Font.Create( "Tuffy16.pak" ) )
        return FALSE;
    m_Font.SetScaleFactors(3.0f, 3.0f);

    InitShaders();

    // Create the shader program needed to render the scene
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( hVertexShader, 1, &g_strVertexShader, NULL );
        glCompileShader( hVertexShader );

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hVertexShader, 1024, &nLength, strLog );
            return FALSE;
        }

        GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( hFragmentShader, 1, &g_strFragmentShader, NULL );
        glCompileShader( hFragmentShader );

        // Check for compile success
        glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hFragmentShader, 1024, &nLength, strLog );
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        g_hShaderProgram = glCreateProgram();
        glAttachShader( g_hShaderProgram, hVertexShader );
        glAttachShader( g_hShaderProgram, hFragmentShader );

        // Init attributes BEFORE linking
        glBindAttribLocation( g_hShaderProgram, g_hVertexLoc,   "g_vPosition" );
        glBindAttribLocation( g_hShaderProgram, g_hColorLoc,    "g_vColor" );

        // Link the vertex shader and fragment shader together
        glLinkProgram( g_hShaderProgram );

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv( g_hShaderProgram, GL_LINK_STATUS, &nLinkResult );
        if( 0 == nLinkResult )
        {
            CHAR strLog[1024];
            GLint nLength;
            glGetProgramInfoLog( g_hShaderProgram, 1024, &nLength, strLog );
            return FALSE;
        }

        // Get uniform locations
        g_hModelViewMatrixLoc = glGetUniformLocation( g_hShaderProgram, "g_matModelView" );
        g_hProjMatrixLoc      = glGetUniformLocation( g_hShaderProgram, "g_matProj" );

        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
    }

    // Create the shader program needed to render the overlay
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( hVertexShader, 1, &g_strOverlayVertexShader, NULL );
        glCompileShader( hVertexShader );

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hVertexShader, 1024, &nLength, strLog );
            return FALSE;
        }

        GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( hFragmentShader, 1, &g_strOverlayFragmentShader, NULL );
        glCompileShader( hFragmentShader );

        // Check for compile success
        glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &nCompileResult );
        if( 0 == nCompileResult )
        {
            CHAR  strLog[1024];
            GLint nLength;
            glGetShaderInfoLog( hFragmentShader, 1024, &nLength, strLog );
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        g_hOverlayShaderProgram = glCreateProgram();
        glAttachShader( g_hOverlayShaderProgram, hVertexShader );
        glAttachShader( g_hOverlayShaderProgram, hFragmentShader );

        // Link the vertex shader and fragment shader together
        glLinkProgram( g_hOverlayShaderProgram );

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv( g_hOverlayShaderProgram, GL_LINK_STATUS, &nLinkResult );
        if( 0 == nLinkResult )
        {
            CHAR strLog[1024];
            GLint nLength;
            glGetProgramInfoLog( g_hOverlayShaderProgram, 1024, &nLength, strLog );
            return FALSE;
        }

        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
    }

    // Create an offscreen FBO to render to
    if( FALSE == CreateFBO( 4096, 4096, GL_RGBA, GL_FLOAT, GL_RGBA16F, &g_pOffscreenFBO0 ) )
        return FALSE;
    if( FALSE == CreateFBO( 4096, 4096, GL_RGBA, GL_FLOAT, GL_RGBA16F, &g_pOffscreenFBO1 ) )
        return FALSE;

    return TRUE;
}



//--------------------------------------------------------------------------------------
// Name: Resize()
// Desc:
//--------------------------------------------------------------------------------------
BOOL CSample::Resize()
{
    glViewport(0, 0, m_nWidth, m_nHeight);
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::Destroy()
{
    glDeleteProgram( g_hShaderProgram );
    DestroyFBO( g_pOffscreenFBO0 );
    DestroyFBO( g_pOffscreenFBO1 );
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

    // Toggle clearALL
    if (pntState & FRM_INPUT::POINTER_RELEASED)
    {
        m_bClearAll = 1 - m_bClearAll;
    }

    return;
}


//--------------------------------------------------------------------------------------
// Name: RenderScene()
// Desc: Render the scene
//--------------------------------------------------------------------------------------
VOID CSample::RenderScene( FLOAT fTime )
{
    // Rotate and translate the model view matrix
    float matModelView[16] = {0};
    matModelView[ 0] = +cosf( fTime );
    matModelView[ 2] = +sinf( fTime );
    matModelView[ 5] = 1.0f;
    matModelView[ 8] = -sinf( fTime );
    matModelView[10] = +cosf( fTime );
    matModelView[14] = -5.0f;
    matModelView[15] = 1.0f;

    // Build a perspective projection matrix
    float matProj[16] = {0};
    matProj[ 0] = cosf( 0.5f ) / sinf( 0.5f );
    matProj[ 5] = matProj[0] * g_fAspectRatio;
    matProj[10] = -( 10.0f ) / ( 9.0f );
    matProj[11] = -1.0f;
    matProj[14] = -( 10.0f ) / ( 9.0f );

    // Define vertice for a 4-sided pyramid
    FLOAT VertexPositions[] =
            {
                    0.0f,+1.0f, 0.0f,      -1.0f,-1.0f, 1.0f,      +1.0f,-1.0f, 1.0f,
                    0.0f,+1.0f, 0.0f,      +1.0f,-1.0f, 1.0f,      +1.0f,-1.0f,-1.0f,
                    0.0f,+1.0f, 0.0f,      +1.0f,-1.0f,-1.0f,      -1.0f,-1.0f,-1.0f,
                    0.0f,+1.0f, 0.0f,      -1.0f,-1.0f,-1.0f,      -1.0f,-1.0f, 1.0f,
            };
    FLOAT VertexColors[] =
            {
                    1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f,       1.0f, 0.0f, 0.0f, // Red
                    0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f,       0.0f, 1.0f, 0.0f, // Green
                    0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 1.0f,       0.0f, 0.0f, 1.0f, // Blue
                    1.0f, 1.0f, 0.0f,       1.0f, 1.0f, 0.0f,       1.0f, 1.0f, 0.0f, // Yellow
            };

    // Clear the colorbuffer and depth-buffer
    if (m_bClearAll)
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );
    else
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set some state
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    // Set the shader program
    glUseProgram( g_hShaderProgram );
    glUniformMatrix4fv( g_hModelViewMatrixLoc, 1, FALSE, matModelView );
    glUniformMatrix4fv( g_hProjMatrixLoc,      1, FALSE, matProj );

    // Bind the vertex attributes
    glVertexAttribPointer( g_hVertexLoc, 3, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( g_hVertexLoc );

    glVertexAttribPointer( g_hColorLoc, 3, GL_FLOAT, 0, 0, VertexColors );
    glEnableVertexAttribArray( g_hColorLoc );

    // Draw the cubemap-reflected triangle
    glDrawArrays( GL_TRIANGLES, 0, 12 );
   // glDrawArrays( GL_TRIANGLES, 0, 12 );
    // Cleanup
    glDisableVertexAttribArray( g_hVertexLoc );
    glDisableVertexAttribArray( g_hColorLoc );
}


//--------------------------------------------------------------------------------------
// Name: Render()
// Desc:
//--------------------------------------------------------------------------------------
VOID CSample::Render()
{
    static float fTime = 0.0f;
    fTime += 0.01f;

    // Render the scene into the offscreen FBO with a green background
    {
        BeginFBO( g_pOffscreenFBO0);
        glClearColor( 0.0f, 0.5f, 0.0f, 1.0f );
        glClearDepthf(1.0);
        RenderScene( fTime );
        EndFBO( g_pOffscreenFBO0 );
    }

    {
        BeginFBO( g_pOffscreenFBO1);
        glClearColor( 0.0f, 0.5f, 0.0f, 1.0f );
        glClearDepthf(1.0);
        RenderScene( fTime );
        EndFBO( g_pOffscreenFBO1 );
    }

    // Render the scene into the primary backbuffer with a dark blue background
    glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
    glClearDepthf(1.0);
    RenderScene( fTime );

    // Display the offscreen FBO texture as an overlay
    {
        float vQuad[] =
                {
                        0.35f, 0.95f,  0.0f, 1.0f,
                        0.35f, 0.35f,  0.0f, 0.0f,
                        0.60f, 0.35f,  1.0f, 0.0f,
                        0.60f, 0.95f,  1.0f, 1.0f,
                };
        glVertexAttribPointer( 0, 4, GL_FLOAT, 0, 0, vQuad );
        glEnableVertexAttribArray( 0 );

        // Set the texture
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, g_pOffscreenFBO0->m_hTexture );
        glDisable( GL_DEPTH_TEST );

        // Set the shader program
        glUseProgram( g_hOverlayShaderProgram );

        // Draw the quad
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        float vQuad1[] =
                {
                        0.70f, 0.95f,  0.0f, 1.0f,
                        0.70f, 0.35f,  0.0f, 0.0f,
                        0.95f, 0.35f,  1.0f, 0.0f,
                        0.95f, 0.95f,  1.0f, 1.0f,
                };
        glVertexAttribPointer( 0, 4, GL_FLOAT, 0, 0, vQuad1 );
        glEnableVertexAttribArray( 0 );

        // Set the texture
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, g_pOffscreenFBO1->m_hTexture );

        // Set the shader program
        glUseProgram( g_hOverlayShaderProgram );

        // Draw the quad
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );
    }

    CHAR strMode[40];

    if (m_bClearAll)
        FrmSprintf( strMode, 40, "ClearAll");
    else
        FrmSprintf( strMode, 40, "ClearColorDepth");

    m_Font.DrawText(m_nWidth/2.0f, 80, FRMCOLOR_RED, strMode, FRM_FONT_CENTER);

}

