// Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "Scene.h"

#include "FrmSpline.h"
#include "FrmUtils.h"
#include <GLES3/gl31.h>
#include <OpenGLES/FrmShader.h>
#include <string>
#include <tinygltf/examples/common/glm/glm/gtc/matrix_transform.hpp>
#include <vector>


#if defined(__linux__)
#include <stdio.h>
#include <unistd.h>
#endif


extern AAssetManager* g_pAssetManager;


//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
CFrmApplication* FrmCreateApplicationInstance()
{
    CSample*const sample = new CSample("GLTF");
    sample->m_nNumStencilBufferBits = 0;//back buffer doesn't perform stencil operations, so save Gpu memory
    return sample;
}


//--------------------------------------------------------------------------------------
// Name: CSample()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CSample::CSample(const CHAR* strName) : CFrmApplication(strName)
{
    m_windowTitle = strName;
    m_windowWidth = 765;
    m_windowHeight = 480;
    m_aspectRatio = (FLOAT)m_windowWidth / (FLOAT)m_windowHeight;

    m_shaderProgram = 0;
    m_vertexLoc = 0;
    m_textureLoc = 1;
    m_normalLoc = 2;
}


//--------------------------------------------------------------------------------------
// Name: InitShaders()
// Desc: Initialize the shaders
//--------------------------------------------------------------------------------------
BOOL CSample::InitShaders()
{
    m_vertexShaderProgram = 
        "attribute vec4 g_vVertex;                                           \n"
        "attribute vec2 g_vVertexTex;                                        \n"
        "attribute vec4 g_vVertexNormal;                                     \n"
        "uniform mat4   g_modelToClip;                                       \n"
        "uniform mat4   g_modelRotationLocal;                                \n"
        "varying vec2   g_vTexCoord;                                         \n"
        "varying vec4   g_vNormal;                                           \n"
        "                                                                    \n"
        "void main()                                                         \n"
        "{                                                                   \n"
        "    gl_Position  = g_modelToClip*vec4( g_vVertex.x, g_vVertex.y,    \n"
        "                                       g_vVertex.z, g_vVertex.w );  \n"
        "    g_vTexCoord = g_vVertexTex;                                     \n"
        "    g_vNormal = g_modelRotationLocal*g_vVertexNormal;               \n"
        "}                                                                   \n";

    m_fragmentShaderProgram =
        "#ifdef GL_FRAGMENT_PRECISION_HIGH                                   \n"
        "   precision highp float;                                           \n"
        "#else                                                               \n"
        "   precision mediump float;                                         \n"
        "#endif                                                              \n"
        "                                                                    \n"
        "uniform sampler2D g_Texture;                                        \n"
        "varying vec2      g_vTexCoord;                                      \n"
        "varying vec4      g_vNormal;                                        \n"
        "void main()                                                         \n"
        "{                                                                   \n"
        "    gl_FragColor = texture2D( g_Texture, g_vTexCoord );             \n"
        "}                                                                   \n";

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CSample::Initialize()
{
    InitShaders();

    const size_t kAttrsNum=3;
    FRM_SHADER_ATTRIBUTE attrs[kAttrsNum];
    attrs[0].strName = "g_vVertex";
    attrs[0].nLocation = m_vertexLoc;
    attrs[1].strName = "g_vVertexTex";
    attrs[1].nLocation = m_textureLoc;
    attrs[2].strName = "g_vVertexNormal";
    attrs[2].nLocation = m_normalLoc;
    if(!FrmCompileShaderProgram(m_vertexShaderProgram, m_fragmentShaderProgram, &m_shaderProgram, &attrs[0], kAttrsNum))
    {
        return FALSE;
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glEnable(GL_DEPTH_TEST);

    m_modelToClipLoc = glGetUniformLocation(m_shaderProgram, "g_modelToClip");
    m_modelRotationLocalLoc = glGetUniformLocation(m_shaderProgram, "g_modelRotationLocal");


    std::vector<Adreno::Model> adrenoModels;
    m_textureFilepaths.push_back(std::string("Textures/white_d.ktx"));//note that if support is added for normal maps, Textures/normal_default.png may become relevant
    m_baseColorDefault_MaterialId = m_textureFilepaths.size();    
    const bool gltfLoadResult = Adreno::FrmLoadGLTFModelsFromTextFile(
        &adrenoModels,
        &m_textureFilepaths, 
        &m_meshesSortedByMaterialId, 
        "Meshes/Museum.gltf",
        m_baseColorDefault_MaterialId);
    ADRENO_ASSERT(gltfLoadResult, __FILE__, __LINE__);

    Adreno::FrmMeshesMergeIfSameMaterial(&m_adrenoModelMerged, m_meshesSortedByMaterialId);
    //non-merged adrenoModels' dynamic memory will be freed upon this function returning in their destructors

    Adreno::FrmKtxFilesLoad(&m_textureHandles, m_textureFilepaths);

    size_t indexBufferHandleIndex = 0;
    const auto& adrenoModel = m_adrenoModelMerged;
    for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex)
    {
        const Adreno::Mesh& mesh = adrenoModel.Meshes[meshIndex];

        const UINT32 meshNumSurfaces = mesh.Surfaces.NumSurfaces;
        if (meshNumSurfaces > 0)
        {
            ADRENO_ASSERT(meshNumSurfaces == 1, __FILE__, __LINE__);
        }

        const Adreno::IndexBuffer adrenoIndexBuffer = mesh.Indices;
        GLuint& indexBufferHandle = m_indexBufferHandles[indexBufferHandleIndex++];
        glGenBuffers(1, &indexBufferHandle);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferHandle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, adrenoIndexBuffer.NumIndices * sizeof(adrenoIndexBuffer.Indices[0]), &adrenoIndexBuffer.Indices[0], GL_STATIC_DRAW);
        //LOGI(   "adrenoIndexBuffer.NumIndices=%u;sizeof(adrenoIndexBuffer.Indices[0])=%zu;&adrenoIndexBuffer.Indices[0]=%p",
        //        adrenoIndexBuffer.NumIndices, sizeof(adrenoIndexBuffer.Indices[0]), &adrenoIndexBuffer.Indices[0]);
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
    glDeleteProgram(m_shaderProgram);
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
    ADRENO_ASSERT(glGetError() == GL_NO_ERROR, __FILE__, __LINE__);

    float timeSinceLastTraditionalRender;
    if (m_framesRendered == 0)
    {
        timeSinceLastTraditionalRender = 0.f;
        Adreno::ClockGetTime(&m_renderStart);
    }
    else
    {
        Adreno::ClockGetTime(&m_renderStop);
        timeSinceLastTraditionalRender = static_cast<float>(Adreno::Seconds(m_renderStop - m_renderStart));
        m_renderStart = m_renderStop;
    }

    //LOGI("timeSinceLastTraditionalRender=%f", timeSinceLastTraditionalRender);

    glm::vec3 splinePoints[kSplinePointsNum];
    splinePoints[0] = glm::vec3(-1.f, 0.f, -1.f);
    splinePoints[1] = glm::vec3(-1.f, 0.f, 1.f);
    splinePoints[2] = glm::vec3(1.f, 0.f, 1.f);
    splinePoints[3] = glm::vec3(1.f, 0.f, -1.f);
    for (size_t i = 0; i < kSplinePointsNum; ++i)
    {
        splinePoints[i] *= 16.f;
        splinePoints[i].y = 15.f;
    }
    const float kSplinePointsNumFloat = static_cast<float>(kSplinePointsNum);
    const glm::vec3 cameraPosition = Adreno::SplineEvaluate(m_splineT, &splinePoints[0], kSplinePointsNum);
    const glm::vec3 cameraPositionLookahead =
        Adreno::SplineEvaluate(Adreno::SplineTIncrement(m_splineT, .0625f, kSplinePointsNumFloat), &splinePoints[0], kSplinePointsNum);
    m_splineT = Adreno::SplineTIncrement(m_splineT, timeSinceLastTraditionalRender * .2f, kSplinePointsNumFloat);


    // Clear the backbuffer and depth-buffer
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(m_shaderProgram);

    const glm::vec3 worldUp(0.f, 1.f, 0.f);
    const glm::mat4 localToWorld = glm::rotate(glm::mat4(), glm::radians(270.f), worldUp);
    const glm::mat4 modelToClip =
        glm::perspective(glm::radians(90.f), 1.f / m_aspectRatio, .4f, 90.f) *
        glm::lookAt(cameraPosition, cameraPositionLookahead, worldUp) *
        glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.f)) *
        localToWorld;
    glUniformMatrix4fv(m_modelToClipLoc, 1, GL_FALSE, &modelToClip[0][0]);
    glUniformMatrix4fv(m_modelRotationLocalLoc, 1, GL_FALSE, &localToWorld[0][0]);


    size_t indexBufferIndex = 0;
    const auto& adrenoModel = m_adrenoModelMerged;
    for (size_t meshIndex = 0; meshIndex < adrenoModel.NumMeshes; ++meshIndex)
    {
        // Draw the model
        const Adreno::Mesh& adrenoMesh = adrenoModel.Meshes[meshIndex];
        const Adreno::MeshSurfaceArray& adrenoMeshSurfaces = adrenoMesh.Surfaces;

        if (adrenoMeshSurfaces.NumSurfaces == 1)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, m_textureHandles[adrenoMeshSurfaces.SurfaceGet(0)->MaterialId]);
        }

        const Adreno::VertexFormat& adrenoVertexFormat = adrenoMesh.Vertices.Format;
        for (   INT32 adrenoVertexFormatPropertiesIndex = 0; 
                adrenoVertexFormatPropertiesIndex < adrenoVertexFormat.NumProperties; 
                ++adrenoVertexFormatPropertiesIndex)
        {
            const Adreno::VertexProperty& vertexProperty = adrenoVertexFormat.Properties[adrenoVertexFormatPropertiesIndex];
            if (vertexProperty.Valid())
            {
                const char* const vertexPropertyName = &vertexProperty.Usage.Name[0];
                const GLuint doNotHandle = UINT32_MAX;
                GLuint loc = doNotHandle;
                if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_POSITION))
                {
                    loc = m_vertexLoc;
                }
                else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_NORMAL))
                {
                    loc = m_normalLoc;
                }
                else if (VertexPropertyIs(vertexProperty.Usage.Name, Adreno::ATTRIB_TEXCOORD_0))
                {
                    loc = m_textureLoc;
                }
                //don't handle other vertex attributes yet

                if (loc != doNotHandle)
                {
                    glVertexAttribPointer(
                        loc,
                        vertexProperty.NumValues(),
                        GL_FLOAT,
                        0,
                        adrenoVertexFormat.Stride,
                        &adrenoMesh.Vertices.Buffer[vertexProperty.Offset]);
                    glEnableVertexAttribArray(loc);
                }
                //LOGI( "%zu/%s:VertexPropertyType=%i,Offset=%u,Size=%u,NumValues=%u,IsNormalized=%i, loc=%i",
                //      adrenoVertexFormatPropertiesIndex, &vertexProperty.Usage.Name[0], vertexProperty.Type, vertexProperty.Offset, vertexProperty.Size(), vertexProperty.NumValues(), vertexProperty.IsNormalized(), loc);                
            }
        }
        // Draw mesh
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBufferHandles[indexBufferIndex++]);
        glDrawElements(
            GL_TRIANGLES,
            adrenoMesh.Indices.NumIndices,
            GL_UNSIGNED_INT,
            NULL);

        glDisableVertexAttribArray(m_vertexLoc);
        glDisableVertexAttribArray(m_textureLoc);
        glDisableVertexAttribArray(m_normalLoc);
    }

    ++m_framesRendered;
}
