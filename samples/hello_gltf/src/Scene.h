// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef SCENE_H
#define SCENE_H

#include "FrmApplication.h"
#include "FrmModelIO.h"
#include "FrmResourceGLES.h"
#include "FrmStdLib.h"


//--------------------------------------------------------------------------------------
// Name: class CSample
// Desc: The main application class for this sample
//--------------------------------------------------------------------------------------
typedef void (GL_APIENTRYP PFNGLQCOMFRAMEEXTRAPOLATION) (GLuint src1, GLuint src2, GLuint output, float scaleFactor);
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
    Adreno::Model                       m_adrenoModelMerged;
    std::vector<std::string>            m_textureFilepaths;
    Adreno::MeshesSortedByMaterialId    m_meshesSortedByMaterialId;
    ResourceHandleArray<1024>           m_indexBufferHandles;
    ResourceHandleArray<1024>           m_textureHandles;
    UINT32                              m_baseColorDefault_MaterialId;

    BOOL InitShaders();

    const CHAR*                         m_windowTitle;
    UINT32                              m_windowWidth;
    UINT32                              m_windowHeight;
    FLOAT                               m_aspectRatio;
    GLuint                              m_shaderProgram;
    GLuint                              m_vertexLoc;
    GLuint                              m_normalLoc;
    GLuint                              m_textureLoc;
    GLuint                              m_modelToClipLoc;
    GLuint                              m_modelRotationLocalLoc;
    const CHAR*                         m_fragmentShaderProgram;
    const CHAR*                         m_vertexShaderProgram;

    timespec                            m_renderStart, m_renderStop;
    size_t                              m_framesRendered = 0;
    const size_t                        kSplinePointsNum = 4;
    float                               m_splineT = static_cast<float>(kSplinePointsNum - 1);
};

#endif // SCENE_H
