// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

#include "FrmFontGLES.h"
#include "FrmUserInterfaceGLES.h"

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
    VOID DrawPyramid(GLenum shadingRate, FLOAT* matModelView, FLOAT* matProj, FLOAT x, FLOAT y, FLOAT z);
    VOID DrawUI();
    
    GLuint m_shaderProgram;
    GLuint m_modelViewMatrixLoc;
    GLuint m_projMatrixLoc;
    GLuint m_vertexLoc;
    GLuint m_uvLoc;
    CHAR* m_pFragmentShader;
    CHAR* m_pVertexShader;
    GLuint m_textureHandle;

    CFrmFontGLES m_font;

    // Variable to check if Shading Rate is available in the current platform
    BOOL m_shadingRateAvailable;
};

#endif // SCENE_H
