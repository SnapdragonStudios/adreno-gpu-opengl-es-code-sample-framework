// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include <FrmPlatform.h>
//#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <FrmApplication.h>
#include <FrmUtils.h>
#include <OpenGLES/FrmShader.h>
#include "Scene.h"

// Define the various parameters we will need to enable and use the shading rate extension.
#ifndef GL_QCOM_shading_rate
#define GL_QCOM_shading_rate 1
#define GL_SHADING_RATE_1X1_PIXELS_QCOM            0x96A6
#define GL_SHADING_RATE_1X2_PIXELS_QCOM            0x96A7
#define GL_SHADING_RATE_2X1_PIXELS_QCOM            0x96A8
#define GL_SHADING_RATE_2X2_PIXELS_QCOM            0x96A9
#define GL_SHADING_RATE_4X2_PIXELS_QCOM            0x96AC
#define GL_SHADING_RATE_4X4_PIXELS_QCOM            0x96AE
typedef void (GL_APIENTRYP PFNGLSHADINGRATEQCOM) (GLenum rate);
PFNGLSHADINGRATEQCOM glShadingRateQCOM;
#endif /* GL_QCOM_shading_rate */

//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    return new CSample( "Transforms" );
}

//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample( const CHAR* strName ) : CFrmApplication( strName )
{
    m_shaderProgram = 0;
    m_vertexLoc = 0;
    m_uvLoc = 1;
    m_shadingRateAvailable = FALSE;
}

//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    m_pVertexShader =
            "#version 300 es													\n"
            "// Declare all the semantics										\n"
            "#define ATTR_POSITION	0											\n"
            "#define ATTR_UV		1											\n"
            "																	\n"
            "layout(location = ATTR_POSITION) in vec4 g_vPosition;				\n"
            "layout(location = ATTR_UV) in vec2 g_vUV;					        \n"
            "                                                                   \n"
            "uniform   mat4 g_matModelView;                                     \n"
            "uniform   mat4 g_matProj;                                          \n"
            "                                                                   \n"
            "out vec2 uv;													    \n"
            "                                                                   \n"
            "void main()														\n"
            "{																	\n"
            "	 vec4 Position = g_vPosition;									\n"
            "    vec4 vPositionES = g_matModelView * (Position);                \n"
            "    gl_Position  = (g_matProj * vPositionES);                      \n"
            "    uv = g_vUV;                                                    \n"
            "}																   	\n";


    m_pFragmentShader =
            "#version 300 es	 												\n"
            "// Declare all the semantics										\n"
            "#define FRAG_COLOR		0											\n"
            "																	\n"
            "#ifdef GL_FRAGMENT_PRECISION_HIGH									\n"
            "   precision highp float;											\n"
            "#else																\n"
            "   precision mediump float;										\n"
            "#endif																\n"
            "																	\n"
            "in vec2 uv;													    \n"
            "uniform   sampler2D g_sImageTexture;                               \n"
            "		                                                            \n"
            "layout(location = FRAG_COLOR) out vec4 Color;						\n"
            "																	\n"
            "void main()														\n"
            "{																	\n"
            "    Color = texture(g_sImageTexture, uv.xy);   					\n"
            "}																	\n";
    
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
    // Try to initialize shading rate
    glShadingRateQCOM = (PFNGLSHADINGRATEQCOM)((void*)eglGetProcAddress("glShadingRateQCOM"));
    if (glShadingRateQCOM != NULL) {
        m_shadingRateAvailable = TRUE;
    }

    InitShaders();

    // Create the Checkerboard texture, loaded from an image file
    {
        UINT32 nWidth, nHeight, nFormat;
        BYTE *pImageData = FrmUtils_LoadTGA("checkerboard.tga", &nWidth, &nHeight, &nFormat);
        if (NULL == pImageData) {
            LOGE("Checkerboard image could not be loaded");
            return FALSE;
        }

        glGenTextures(1, &m_textureHandle);
        glBindTexture(GL_TEXTURE_2D, m_textureHandle);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, nFormat, nWidth, nHeight,
                     0, nFormat, GL_UNSIGNED_BYTE, pImageData);
        delete[] pImageData;
    }

    // Create the shader program needed to render the scene
    {
        // Compile the shaders
        GLuint hVertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(hVertexShader, 1, &m_pVertexShader, NULL);
        glCompileShader(hVertexShader);

        // Check for compile success
        GLint nCompileResult = 0;
        glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &nCompileResult);
        if (0 == nCompileResult) {
            CHAR strLog[1024];
            GLint nLength;
            glGetShaderInfoLog(hVertexShader, 1024, &nLength, strLog);
            return FALSE;
        }

        GLuint hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(hFragmentShader, 1, &m_pFragmentShader, NULL);
        glCompileShader(hFragmentShader);

        // Check for compile success
        glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &nCompileResult);
        if (0 == nCompileResult) {
            CHAR strLog[1024];
            GLint nLength;
            glGetShaderInfoLog(hFragmentShader, 1024, &nLength, strLog);
            return FALSE;
        }

        // Attach the individual shaders to the common shader program
        m_shaderProgram = glCreateProgram();
        glAttachShader(m_shaderProgram, hVertexShader);
        glAttachShader(m_shaderProgram, hFragmentShader);

        // Init attributes BEFORE linking
        glBindAttribLocation(m_shaderProgram, m_vertexLoc, "g_vPosition");
        glBindAttribLocation(m_shaderProgram, m_uvLoc, "g_vUV");

        // Link the vertex shader and fragment shader together
        glLinkProgram(m_shaderProgram);

        // Check for link success
        GLint nLinkResult = 0;
        glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &nLinkResult);
        if (0 == nLinkResult) {
            CHAR strLog[1024];
            GLint nLength;
            glGetProgramInfoLog(m_shaderProgram, 1024, &nLength, strLog);
            return FALSE;
        }

        // Get uniform locations
        m_modelViewMatrixLoc = glGetUniformLocation(m_shaderProgram, "g_matModelView");
        m_projMatrixLoc = glGetUniformLocation(m_shaderProgram, "g_matProj");

        glDeleteShader(hVertexShader);
        glDeleteShader(hFragmentShader);
    }

    if (m_font.Create("Tuffy16.pak"))
    {
        m_font.SetScaleFactors(3.0f, 3.0f);
        LOGI("Font loaded correctly!");
    } else
    {
        LOGE("Font could not be loaded");
        return FALSE;
    }

    // Set some state
    glEnable( GL_DEPTH_TEST );
    glEnable( GL_CULL_FACE );
    glCullFace( GL_BACK );

    return TRUE;
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
    glDeleteTextures( 1, &m_textureHandle );
    glDeleteProgram( m_shaderProgram );
    m_font.Destroy();
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
    static float fAngle = 0.0f;
    fAngle += 0.01f;

    // Define vertices and uvs for a 4-sided pyramid
    FLOAT VertexPositions[] =
    {
        0.0f,+1.0f, 0.0f,      +0.7f,-1.0f, 0.7f,      +0.7f,-1.0f,-0.7f,
        0.0f,+1.0f, 0.0f,      -0.7f,-1.0f, 0.7f,      +0.7f,-1.0f, 0.7f,
        0.0f,+1.0f, 0.0f,      +0.7f,-1.0f,-0.7f,      -0.7f,-1.0f,-0.7f,
        0.0f,+1.0f, 0.0f,      -0.7f,-1.0f,-0.7f,      -0.7f,-1.0f, 0.7f,
    };
    FLOAT VertexUVs[] =
    {
        0.5f, 0.0f,            0.0f, 1.0f,        1.0f, 1.0f,
        0.5f, 0.0f,            0.0f, 1.0f,        1.0f, 1.0f,
        0.5f, 0.0f,            0.0f, 1.0f,        1.0f, 1.0f,
        0.5f, 0.0f,            0.0f, 1.0f,        1.0f, 1.0f,
    };

    // Clear the colorbuffer and depth-buffer
    // Note that  clearing to an arbitrary color is not recommended and it is only done in this
    // sample for presentation purposes. Clearing to black allows the GPU to do a fast clear.
    glClearColor( 0.0f, 0.0f, 0.5f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // Set the shader program
    glUseProgram( m_shaderProgram );
    glBindTexture( GL_TEXTURE_2D, m_textureHandle );

    // Bind the vertex attributes
    glVertexAttribPointer( m_vertexLoc, 3, GL_FLOAT, 0, 0, VertexPositions );
    glEnableVertexAttribArray( m_vertexLoc );

    glVertexAttribPointer( m_uvLoc, 2, GL_FLOAT, 0, 0, VertexUVs );
    glEnableVertexAttribArray( m_uvLoc );

    // Rotate and translate the model view matrix
    float matModelView[16] = {0};
    matModelView[ 0] = +cosf( fAngle );
    matModelView[ 2] = +sinf( fAngle );
    matModelView[ 5] = 1.0f;
    matModelView[ 8] = -sinf( fAngle );
    matModelView[10] = +cosf( fAngle );
    matModelView[15] = 1.0f;

    // Build a simple orthographic projection matrix
    float matProj[16] = {0};
    matProj[0] = 1.0f / 2.0f;
    matProj[5] = 1.0f / 3.0f;
    matProj[10] = -1.0f / 5.0f;
    matProj[12] = matProj[13] = matProj[14] = -1.0f;
    matProj[15] = 1.0f;

    // Draw pyramids with all the possible shading rates
    DrawPyramid(GL_SHADING_RATE_1X1_PIXELS_QCOM, matModelView, matProj, 1.0f, 5.0f, -5.0f);
    DrawPyramid(GL_SHADING_RATE_1X2_PIXELS_QCOM, matModelView, matProj, 3.0f, 5.0f, -5.0f);
    DrawPyramid(GL_SHADING_RATE_2X1_PIXELS_QCOM, matModelView, matProj, 1.0f, 3.0f, -5.0f);
    DrawPyramid(GL_SHADING_RATE_2X2_PIXELS_QCOM, matModelView, matProj, 3.0f, 3.0f, -5.0f);
    DrawPyramid(GL_SHADING_RATE_4X2_PIXELS_QCOM, matModelView, matProj, 1.0f, 1.0f, -5.0f);
    DrawPyramid(GL_SHADING_RATE_4X4_PIXELS_QCOM, matModelView, matProj, 3.0f, 1.0f, -5.0f);

    DrawUI();

    // Cleanup
    glDisableVertexAttribArray( m_vertexLoc );
    glDisableVertexAttribArray( m_uvLoc );
}

//--------------------------------------------------------------------------------------
// Name: DrawPyramid()
// Desc: Draws a pyramid with the specifed shading rate at the desired location and rotation
//--------------------------------------------------------------------------------------
VOID CSample::DrawPyramid(GLenum shadingRate, FLOAT* matModelView, FLOAT* matProj, FLOAT x, FLOAT y, FLOAT z)
{
    // Set position of pyramid
    matModelView[12] = x;
    matModelView[13] = y;
    matModelView[14] = z;

    glUniformMatrix4fv( m_modelViewMatrixLoc, 1, FALSE, matModelView );
    glUniformMatrix4fv( m_projMatrixLoc,      1, FALSE, matProj );

    // If Shading Rate available, apply the corresponding shading rate.
    if (m_shadingRateAvailable) {
        glShadingRateQCOM(shadingRate);
    }

    glDrawArrays( GL_TRIANGLES, 0, 12 );
}

//--------------------------------------------------------------------------------------
// Name: DrawUI()
// Desc: Draw some UI overlay to mark the shading rate used for each pyramid, note that FrmUserInterfaceGLES
//       could be used instead of directly rendering the text.
//--------------------------------------------------------------------------------------
VOID CSample::DrawUI()
{
    m_font.DrawText(m_nWidth/4.0f, m_nHeight/6.0f, FRMCOLOR_GREEN, "1x1", FRM_FONT_CENTER);
    m_font.DrawText(m_nWidth*3.0f/4.0f, m_nHeight/6.0f, FRMCOLOR_GREEN, "1x2", FRM_FONT_CENTER);
    m_font.DrawText(m_nWidth/4.0f, m_nHeight*3.0f/6.0f, FRMCOLOR_GREEN, "2x1", FRM_FONT_CENTER);
    m_font.DrawText(m_nWidth*3.0f/4.0f, m_nHeight*3.0f/6.0f, FRMCOLOR_GREEN, "2x2", FRM_FONT_CENTER);
    m_font.DrawText(m_nWidth/4.0f, m_nHeight*5.0f/6.0f, FRMCOLOR_GREEN, "4x2", FRM_FONT_CENTER);
    m_font.DrawText(m_nWidth*3.0f/4.0f, m_nHeight*5.0f/6.0f, FRMCOLOR_GREEN, "4x4", FRM_FONT_CENTER);

    if (m_shadingRateAvailable) {
        m_font.DrawText(m_nWidth/2.0f, 80, FRMCOLOR_WHITE, "Shading Rate Active", FRM_FONT_CENTER);
    } else {
        m_font.DrawText(m_nWidth/2.0f, 80, FRMCOLOR_RED, "Shading Rate Not Available!", FRM_FONT_CENTER);
    }
}
