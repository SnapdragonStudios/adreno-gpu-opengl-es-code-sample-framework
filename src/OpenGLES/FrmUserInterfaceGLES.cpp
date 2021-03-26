// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "OpenGLES/FrmUserInterfaceGLES.h"
#include "OpenGLES/FrmShader.h"
#include "OpenGLES/FrmUtilsGLES.h"


//--------------------------------------------------------------------------------------
// Name: CFrmUserInterface()
// Desc: CFrmUserInterface constructor
//--------------------------------------------------------------------------------------
CFrmUserInterfaceGLES::CFrmUserInterfaceGLES()
{
    m_hOverlayShader      = 0;
    m_hBackgroundShader   = 0;
    m_hBackgroundColorLoc = 0;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmUserInterface()
// Desc: CFrmUserInterface destructor
//--------------------------------------------------------------------------------------
CFrmUserInterfaceGLES::~CFrmUserInterfaceGLES()
{
    if( m_hBackgroundShader ) glDeleteProgram( m_hBackgroundShader );
    if( m_hOverlayShader )    glDeleteProgram( m_hOverlayShader );
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize the user interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterfaceGLES::Initialize( CFrmFont* pFont, const CHAR* strHeading )
{
    if( !CFrmUserInterface::Initialize( pFont, strHeading ) )
    {
        return FALSE;
    }
    // Create the background shader
    {
#ifdef CORE_GL_CONTEXT
        const CHAR* strBackgroundVertexShader =			
			"#version 300 es \n"
            "in vec2 g_vVertex;                      \n"
            "                                               \n"
            "void main()                                    \n"
            "{                                              \n"
            "    gl_Position = vec4( g_vVertex, 0.0, 1.0 ); \n"
            "}                                              \n";

        const CHAR* strBackgroundFragmentShader =			
			"#version 300 es \n"
            "#ifdef GL_FRAGMENT_PRECISION_HIGH              \n"
            "   precision highp float;                      \n"
            "#else                                          \n"
            "   precision mediump float;                    \n"
            "#endif                                         \n"
            "uniform vec4 g_vBackgroundColor;               \n"
            "out vec4 g_FragColor;                         \n"
            "void main()                                    \n"
            "{                                              \n"
            "    g_FragColor = g_vBackgroundColor;         \n"
            "}                                              \n";                   
#else
        const CHAR* strBackgroundVertexShader =
            "attribute vec2 g_vVertex;                      \n"
            "                                               \n"
            "void main()                                    \n"
            "{                                              \n"
            "    gl_Position = vec4( g_vVertex, 0.0, 1.0 ); \n"
            "}                                              \n";

        const CHAR* strBackgroundFragmentShader =
            "#ifdef GL_FRAGMENT_PRECISION_HIGH              \n"
            "   precision highp float;                      \n"
            "#else                                          \n"
            "   precision mediump float;                    \n"
            "#endif                                         \n"
            "uniform vec4 g_vBackgroundColor;               \n"
            "                                               \n"
            "void main()                                    \n"
            "{                                              \n"
            "    gl_FragColor = g_vBackgroundColor;         \n"
            "}                                              \n";                   

#endif
        if( FALSE == FrmCompileShaderProgram( strBackgroundVertexShader,
                                              strBackgroundFragmentShader, 
                                              &m_hBackgroundShader ) )
        {
            return FALSE;
        }

        m_hBackgroundColorLoc = glGetUniformLocation( m_hBackgroundShader, "g_vBackgroundColor" );
    }

    // Create the overlay shader
    {
#ifdef CORE_GL_CONTEXT
        const CHAR* strOverlayVertexShader =
			"#version 300 es \n" 
            "uniform   vec2 g_vScreenSize;                                           \n"
            "in vec4 g_vVertex;                                               \n"
            "out   vec2 g_vTexCoord;                                             \n"
            "														                 \n"
            "void main()															 \n"
            "{																		 \n"
            "    gl_Position.x = +( 2.0 * ( g_vVertex.x / g_vScreenSize.x ) - 1.0 ); \n"
            "    gl_Position.y = -( 2.0 * ( g_vVertex.y / g_vScreenSize.y ) - 1.0 ); \n"
            "    gl_Position.z = 0.0;                                                \n"
            "    gl_Position.w = 1.0;                                                \n"
            "                                                                        \n"
            "    g_vTexCoord.x = g_vVertex.z;                                        \n"
            "    g_vTexCoord.y = g_vVertex.w;                                        \n"
            "}                                                                       \n";

        const CHAR* strOverlayFragmentShader =
			"#version 300 es \n" 
            "#ifdef GL_FRAGMENT_PRECISION_HIGH						 \n"
            "   precision highp float;					             \n"
            "#else													 \n"
            "   precision mediump float;					         \n"
            "#endif							                         \n"
            "uniform sampler2D g_Texture;                            \n"
            "in vec2      g_vTexCoord;                              \n"
			"out vec4 g_FragColor;                                  \n"
            "                                                        \n"
            "void main()                                             \n"
            "{                                                       \n"
            "    g_FragColor = texture( g_Texture, g_vTexCoord ); \n"
            "}                                                       \n";
#else
        const CHAR* strOverlayVertexShader =
            "uniform   vec2 g_vScreenSize;                                           \n"
            "attribute vec4 g_vVertex;                                               \n"
            "varying   vec2 g_vTexCoord;                                             \n"
            "														                 \n"
            "void main()															 \n"
            "{																		 \n"
            "    gl_Position.x = +( 2.0 * ( g_vVertex.x / g_vScreenSize.x ) - 1.0 ); \n"
            "    gl_Position.y = -( 2.0 * ( g_vVertex.y / g_vScreenSize.y ) - 1.0 ); \n"
            "    gl_Position.z = 0.0;                                                \n"
            "    gl_Position.w = 1.0;                                                \n"
            "                                                                        \n"
            "    g_vTexCoord.x = g_vVertex.z;                                        \n"
            "    g_vTexCoord.y = g_vVertex.w;                                        \n"
            "}                                                                       \n";

        const CHAR* strOverlayFragmentShader =
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

#endif
        if( FALSE == FrmCompileShaderProgram( strOverlayVertexShader,
                                              strOverlayFragmentShader, 
                                              &m_hOverlayShader ) )
        {
            return FALSE;
        }

        m_hOverlayScreenSizeLoc = glGetUniformLocation( m_hOverlayShader, "g_vScreenSize" );
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: RenderOverlays()
// Desc: Renders any overlays attached to the user interface
//--------------------------------------------------------------------------------------
VOID CFrmUserInterfaceGLES::RenderOverlays()
{
    for( UINT32 nOverlayIndex = 0; nOverlayIndex < m_nNumOverlays; ++nOverlayIndex )
    {
        FRM_UI_OVERLAY_INFO* pOverlayInfo = m_pOverlays[ nOverlayIndex ];
        FrmRenderTextureToScreen_GLES( pOverlayInfo->m_fX,  pOverlayInfo->m_fY,
                                       pOverlayInfo->m_fWidth,  pOverlayInfo->m_fHeight,
                                       pOverlayInfo->m_hTextureHandle,
                                       m_hOverlayShader, m_hOverlayScreenSizeLoc );
    }
}


//--------------------------------------------------------------------------------------
// Name: RenderFadedBackground()
// Desc: Fades the background to make the help text more legible.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterfaceGLES::RenderFadedBackground()
{
    // Set the shader program
    glUseProgram( m_hBackgroundShader );
    glUniform4f( m_hBackgroundColorLoc, 0.0f, 0.0f, 0.0f, 0.5f );
    const FLOAT32 Quad[] =
    {
        -1.0, -1.0f,
        +1.0, -1.0f,
        -1.0, +1.0f,
        +1.0, +1.0f,
    };

#if CORE_GL_CONTEXT
	GLuint vao;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(FLOAT32) * 8, Quad, GL_DYNAMIC_DRAW);	
	glVertexAttribPointer( 0, 2, GL_FLOAT, 0, 0, 0 );
#else
	glVertexAttribPointer( 0, 2, GL_FLOAT, 0, 0, Quad );
#endif

    
    glEnableVertexAttribArray( 0 );

    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );

    glDisable( GL_DEPTH_TEST );
    glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
    glEnable( GL_DEPTH_TEST );

    glDisable( GL_BLEND );
	glDisableVertexAttribArray(0);

#if CORE_GL_CONTEXT
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
#endif 
}


