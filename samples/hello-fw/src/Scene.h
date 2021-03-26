// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

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

    const CHAR*  g_strWindowTitle;
    UINT32 g_nWindowWidth;
    UINT32 g_nWindowHeight;
    FLOAT  g_fAspectRatio;
    GLuint       g_hShaderProgram;
    GLuint       g_VertexLoc;
    GLuint       g_ColorLoc;
    CHAR* g_strFSProgram;
    CHAR* g_strVSProgram;
    FRMMATRIX4X4       m_matProj;
};

#endif // SCENE_H
