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

#define THREE_PASS_MODE         0   //Use SGSR 2-pass mode
#define TWO_PASS_MODE           1   //Use SGSR 3-pass mode
#define SWITCH_PASS_MODE        2   //Switch between SGSR 2/3-pass, pass SWITCH_PASS_MODE to initialize resources that both passes needed

namespace SGSR2_Comp
{

const int PIPELINE_COUNT                                            =                3;
const int  RESOURCE_COUNT                                           =                12;

const int  SGSR2_RESOURCE_IDENTIFIER_NULL                           =                -1;
const int  SGSR2_RESOURCE_IDENTIFIER_INPUT_OPAQUE_COLOR             =                0;
const int  SGSR2_RESOURCE_IDENTIFIER_INPUT_COLOR                    =                1;
const int  SGSR2_RESOURCE_IDENTIFIER_INPUT_DEPTH                    =                2;
const int  SGSR2_RESOURCE_IDENTIFIER_INPUT_VELOCITY                 =                3;
const int  SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_ALPHA_BUFFER      =                4;
const int  SGSR2_RESOURCE_IDENTIFIER_YCoCgCOLOR                     =                5;
const int  SGSR2_RESOURCE_IDENTIFIER_MOTION_DEPTH_CLIP_ALPHA_BUFFER =                6;
const int  SGSR2_RESOURCE_IDENTIFIER_PREV_LUMA_HISTORY              =                7;
const int  SGSR2_RESOURCE_IDENTIFIER_LUMA_HISTORY                   =                8;
const int  SGSR2_RESOURCE_IDENTIFIER_PREV_HISTORY_OUTPUT            =                9;
const int  SGSR2_RESOURCE_IDENTIFIER_HISTORY_OUTPUT                 =                10;
const int  SGSR2_RESOURCE_IDENTIFIER_UPSCALED_OUTPUT                =                11;

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
    Resource                   opaqueColorTexture;  /**< a Resource structure containing the handle and information of texture without transparent objects */
    Resource                   colorTexture;        /**< a Resource structure containing the handle and information of texture with transparent objects */
    Resource                   depthTexture;        /**< a Resource structure containing the handle and information of depth texture */
    Resource                   velocityTexture;     /**< a Resource structure containing the handle and information of velocity texture, set it as dummy if no moving objects */
    Resource                   outputTexture;       /**< a Resource structure containing the handle and information of the output upscaled texture */
    uint32_t                   renderSize[2];       /**< render size */
    uint32_t                   displaySize[2];      /**< display size */
    float                      renderSizeRcp[2];    /**< reciprocal of render size */
    float                      displaySizeRcp[2];   /**< reciprocal of display size */
    float                      jitterOffset[2];
    float                      clipToPrevClip[16];  /**< matrix of converting current clip space position to previous clip scape postion */
    float                      preExposure;         /**< exposure used to do tone mapping, it is calculated as View.PreExposure / View.PrevViewInfo.SceneColorPreExposure. */
    float                      cameraFovAngleHor;   /**< tangent of the half horizontal FOV angle */
    float                      cameraNear;          /**< near plane of camera */
    float                      MinLerpContribution;
    uint32_t                   sameCameraFrmNum;    /**< the frame number of camera keeping still */
    uint32_t                   reset;               /**< if it's CameraCut */
    uint8_t                    mode;                /**< SGSR2 algorithm mode: 0 for 3-pass, 1 for 2-pass, 3 for switching between */
} DispatchDescription;

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
        //For 3-pass algorithm
        SGSR2_PASS_CONVERT  = 0,
        SGSR2_PASS_ACTIVATE = 1,
        SGSR2_PASS_UPSCALE   = 2,

        //For 2-pass algorithm
        SGSR2_PASS_CONVERT2PASS  = 3,
        SGSR2_PASS_UPSCALE2PASS   = 4,

        SGSR2_PASS_NONE  = 5,
    } SgsrPass;

    /// A single shader blob and a description of its resources.
    typedef struct ShaderBlob {
        std::string  cShaderStr;               // A pointer to the blob
        uint32_t      shaderHandle;
    } ShaderBlob;

    /// A structure encapsulating a single pass of an algorithm.
    typedef struct Pipeline{
        uint32_t         program;                                        ///< The pipeline object
    } Pipeline;

    /// A structure describing a compute render job.
    typedef struct ComputeJobDescription {
        Pipeline       pipeline;                               ///< Compute pipeline for the render job.
        uint32_t       dimensions[3];                          ///< Dispatch dimensions.

    } ComputeJobDescription;

    typedef struct Sgsr2Ubo {
        glm::uvec2                  renderSize;
        glm::uvec2                  displaySize;
        glm::vec2                   renderSizeRcp;
        glm::vec2                   displaySizeRcp;
        glm::vec2                   jitterOffset;
        glm::vec4                   clipToPrevClip[4];
        float                       preExposure;
        float                       cameraFovAngleHor;
        float                       cameraNear;
        float                       MinLerpContribution;
        uint32_t                    bSameCamera;
        uint32_t                    reset;
    } Sgsr2Ubo;

    glutil::Buffer        m_Sgsr2Ubo{};
    Resource              resources[RESOURCE_COUNT];
    uint32_t                m_NearestClamp;
    uint32_t                m_LinearRepeat;
    uint32_t                m_LinearClamp;
    uint32_t  m_FrameIdx = 0;
    uint8_t   m_Mode = THREE_PASS_MODE;
    //Pipelines for three-pass algorithm
    Pipeline pipelineConvert;
    Pipeline pipelineActivate;
    Pipeline pipelineUpscale;
    //Pipelines for two-pass algorithm
    Pipeline pipelineConvert2Pass;
    Pipeline pipelineUpscale2Pass;

    ComputeJobDescription computeJobDescriptionConvert;
    ComputeJobDescription computeJobDescriptionActivate;
    ComputeJobDescription computeJobDescriptionUpscale;

    void CreateShaderBlobFromPass(SgsrPass pass, ShaderBlob *shaderBlob);
    void CreateResource(const CreateResourceDescription *createResourceDescription);
    void DestroyResources();

    void CreatePipeline(SgsrPass pass, Pipeline* outPipeline);
    void DestroyPipelines();
    void ScheduleJobCompute(const Pipeline pipeline, ComputeJobDescription* computeJobDescriptions,uint32_t dispatchX, uint32_t dispatchY, uint32_t dispatchZ);
    void ExecuteJobCompute(SgsrPass pass, ComputeJobDescription* computeJobDescriptions);

public:
    void RegisterResource(const Resource* inResource, uint32_t internalId);
    void Init(uint32_t inputWidth, uint32_t inputHeight, uint32_t outputWidth, uint32_t outputHeight, uint8_t algoMode);
    void ExecuteSGSR2(DispatchDescription* description);
    void Destroy();

};
};