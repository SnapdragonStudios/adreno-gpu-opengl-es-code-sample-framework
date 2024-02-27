// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
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
    const size_t kAttrsNum=2;
    FRM_SHADER_ATTRIBUTE attrs[kAttrsNum];
    attrs[0].strName = "g_vVertex";
    attrs[0].nLocation = g_VertexLoc;
    attrs[1].strName = "g_vColor";
    attrs[1].nLocation = g_ColorLoc;
    if(!FrmCompileShaderProgram(g_strVSProgram, g_strFSProgram, &g_hShaderProgram, &attrs[0], kAttrsNum))
    {
        return FALSE;
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

