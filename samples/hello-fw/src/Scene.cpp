// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <FrmPlatform.h>
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <FrmApplication.h>
#include <OpenGLES/FrmShader.h>
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
    return new CSample( "DrawTriangle" );
}


//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample( const CHAR* strName ) : CFrmApplication( strName )
{
    g_strWindowTitle = strName;
    g_nWindowWidth   = 765;
    g_nWindowHeight  = 480;
    g_fAspectRatio   = (FLOAT)g_nWindowWidth / (FLOAT)g_nWindowHeight;

    g_hShaderProgram = 0;
    g_VertexLoc = 0;
    g_ColorLoc  = 1;
}


//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    g_strVSProgram =
            "attribute vec4 g_vVertex;											 \n"
            "attribute vec4 g_vColor;											 \n"
            "varying   vec4 g_vVSColor;                                          \n"
            "																	 \n"
            "void main()														 \n"
            "{																	 \n"
            "    gl_Position  = vec4( g_vVertex.x, g_vVertex.y,                  \n"
            "                         g_vVertex.z, g_vVertex.w );                \n"
            "    g_vVSColor = g_vColor;                                          \n"
            "}																	 \n";

    g_strFSProgram =
            "#ifdef GL_FRAGMENT_PRECISION_HIGH									 \n"
            "   precision highp float;											 \n"
            "#else																 \n"
            "   precision mediump float;										 \n"
            "#endif																 \n"
            "																	 \n"
            "varying   vec4 g_vVSColor;                                          \n"
            "																	 \n"
            "void main()														 \n"
            "{																	 \n"
            "    gl_FragColor = g_vVSColor;                                      \n"
            "}																	 \n";

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{

    InitShaders();

    // Create the shader program needed to render the texture
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader( GL_VERTEX_SHADER );
        glShaderSource( hVertexShader, 1, &g_strVSProgram, NULL );
        glCompileShader( hVertexShader );

        GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
        glShaderSource( hFragmentShader, 1, &g_strFSProgram, NULL );
        glCompileShader( hFragmentShader );

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &nCompileResult);
        if (!nCompileResult)
        {
            CHAR Log[1024];
            GLint nLength;
            glGetShaderInfoLog(hFragmentShader, 1024, &nLength, Log);
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        g_hShaderProgram = glCreateProgram();
        glAttachShader( g_hShaderProgram, hVertexShader );
        glAttachShader( g_hShaderProgram, hFragmentShader );

        // Init attributes BEFORE linking
        glBindAttribLocation(g_hShaderProgram, g_VertexLoc, "g_vVertex");
        glBindAttribLocation(g_hShaderProgram, g_ColorLoc, "g_vColor");

        // Link the vertex shader and fragment shader together
        glLinkProgram( g_hShaderProgram );

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv(g_hShaderProgram, GL_LINK_STATUS, &nLinkResult);
        if (!nLinkResult)
        {
            CHAR Log[1024];
            GLint nLength;
            glGetProgramInfoLog(g_hShaderProgram, 1024, &nLength, Log);
            return FALSE;
        }

        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
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
    glDeleteProgram( g_hShaderProgram );
}


//--------------------------------------------------------------------------------------
// Name: Update()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Update()
{
    return;
}


//--------------------------------------------------------------------------------------
// Name: Render()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CSample::Render()
{
    FLOAT fSize = 0.5f;
    FLOAT VertexPositions[] =
            {
                    0.0f,  +fSize*g_fAspectRatio, 0.0f, 1.0f,
                    -fSize, -fSize*g_fAspectRatio, 0.0f, 1.0f,
                    +fSize, -fSize*g_fAspectRatio, 0.0f, 1.0f,
            };

    FLOAT VertexColors[] = {1.0f, 0.0f, 0.0f, 1.0f,
                            0.0f, 1.0f, 0.0f, 1.0f,
                            0.0f, 0.0f, 1.0f, 1.0f
    };

    // Clear the backbuffer and depth-buffer
    glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set the shader program and the texture
    glUseProgram( g_hShaderProgram );

    // Draw the colored triangle
    glVertexAttribPointer( g_VertexLoc, 4, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( g_VertexLoc );

    glVertexAttribPointer( g_ColorLoc, 4, GL_FLOAT, 0, 0, VertexColors);
    glEnableVertexAttribArray( g_ColorLoc );

    glDrawArrays( GL_TRIANGLE_STRIP, 0, 3 );

    glDisableVertexAttribArray( g_VertexLoc );
    glDisableVertexAttribArray( g_ColorLoc );
}

