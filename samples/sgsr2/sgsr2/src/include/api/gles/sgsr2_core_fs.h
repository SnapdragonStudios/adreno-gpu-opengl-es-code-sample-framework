//============================================================================================================
//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#pragma once
#include <string>
#include <glm/glm.hpp>
#include "api/gles/sgsr2_util.h"
namespace SGSR2_Frag
{
const int PIPELINE_COUNT                                               =            3;
const int RESOURCE_COUNT                                               =            7;

const int SGSR2_RESOURCE_IDENTIFIER_NULL                               =            -1;
const int SGSR2_RESOURCE_IDENTIFIER_INPUT_OPAQUE_COLOR                 =            0;
const int SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR                        =            1;
const int SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH                        =            2;
const int SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY                     =            3;
const int SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER     =            4;
const int SGSR2_RESOURCE_IDENTIFIER_PREV_OUTPUT                        =            5;        ///< For the optimized 2-pass-fs mode, the order of PREV_OUTPUT and OUTPUT is important for double buffer
const int SGSR2_RESOURCE_IDENTIFIER_OUTPUT                             =            6;        ///< For the optimized 2-pass-fs mode

typedef struct ResourceDescription {
    std::string                 name;
    void*                       format;
    uint32_t                    width;
    uint32_t                    height;
} ResourceDescription;

typedef struct Resource
{
    uint resource;
    ResourceDescription  resourceDescription;
} Resource;

typedef struct DispatchDescription {
    Resource                   colorTexture;        /**< a Resource structure containing the handle and information of texture with transparent objects */
    Resource                   depthTexture;        /**< a Resource structure containing the handle and information of depth texture */
    Resource                   velocityTexture;     /**< a Resource structure containing the handle and information of velocity texture, set it as dummy if no moving objects */
    Resource                   outputTexture;       /**< a Resource structure containing the handle and information of the output upscaled texture */
    float                      clipToPrevClip[16];  /**< matrix of converting current clip space position to previous clip scape postion */
    float                      renderSize[2];       /**< render size */
    float                      outputSize[2];      /**< output size */
    float                      renderSizeRcp[2];    /**< reciprocal of render size */
    float                      outputSizeRcp[2];   /**< reciprocal of display size */
    float                      jitterOffset[2];
    float                      scaleRatio[2];      /**< viewportXScale & scalefactor */
    float                      cameraFovAngleHor;  /**< the tangent of the half horizontal FOV angle */
    float                      minLerpContribution;
    float                      reset;               /**< if it's cameracut */
    uint32_t                   sameCameraFrmNum;         /**< the frame number of camera keeping still */
} SGSR2ResourceDescription;

class SGSR2_Core{
private:
    /// A structure containing the data required to create a resource.
    typedef struct CreateResourceDescription {
        ResourceDescription          resourceDescription;                    ///< A resource description.
        std::string                  name;                                   ///< Name of the resource.
        uint32_t                     internalId;                             ///< Internal resource ID.
        unsigned char*               initData;                               ///< Buffer containing data to fill the resource.
    } CreateResourceDescription;

    /// An enumeration of resource usage.
    typedef enum SgsrPass {
        //For 2-pass fragment algorithm
        SGSR2_PASS_CONVERT2FRAG  = 0,
        SGSR2_PASS_UPSCALE2FRAG   = 1,

        SGSR2_PASS_NONE  = 2,
    } SgsrPass;

    /// A single shader blob and a description of its resources.
    typedef struct ShaderBlob {
        std::string  cShaderStr;               // A pointer to the blob
        GLuint       shaderHandle;
    } ShaderBlob;

    /// A structure encapsulating a single pass of an algorithm.
    typedef struct Pipeline{
        GLuint         program;                                        ///< The pipeline object
    } Pipeline;

    typedef struct Sgsr2Ubo {
        glm::vec4                   clipToPrevClip[4];
        glm::vec2                   renderSize;
        glm::vec2                   outputSize;
        glm::vec2                   renderSizeRcp;
        glm::vec2                   outputSizeRcp;
        glm::vec2                   jitterOffset;
        glm::vec2                   scaleRatio;
        float                       cameraFovAngleHor;
        float                       minLerpContribution;
        float                       reset;
        uint32_t                    bSameCamera;
    } Sgsr2Ubo;

    glutil::Buffer          m_Sgsr2Ubo{};
    Resource                resources[RESOURCE_COUNT];
    GLuint                  m_NearestClamp;
    GLuint                  m_LinearClamp;
    uint32_t                m_FrameIdx = 0;

    //FrameBuffer for 2-pass fragment shader
    uint32_t                m_ConvertFBHandle = 0;
    uint32_t                m_UpscaleFBHandle = 0;

    //Pipelines for two-pass fragment shader algorithm
    Pipeline                pipelineConvert2Frag;
    Pipeline                pipelineUpscale2Frag;
    uint                    m_VAO;
    uint                    m_VBO;

    void CreateShaderBlobFromPass(SgsrPass pass, ShaderBlob *shaderBlob);
    void CreateResource(const CreateResourceDescription *createResourceDescription);
    void DestroyResources();

    void CreatePipeline(SgsrPass pass, Pipeline* outPipeline);
    void DestroyPipelines();
    void ExecuteJobGraphics(SgsrPass pass, Sgsr2Ubo &ubo);
    void initVertexAttribute();

public:
    void RegisterResource(const Resource* inResource, uint32_t internalId);
    void Init(uint32_t inputWidth, uint32_t inputHeight, uint32_t outputWidth, uint32_t outputHeight);
    void ExecuteSGSR2(SGSR2ResourceDescription* description);
    void Destroy();

};
};