//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_MODEL_IO_H
#define ADRENO_MODEL_IO_H

#include <unordered_map>
#include <string>
#include <vector>

#include "FrmPlatform.h"
#include "FrmAnimation.h"
#include "FrmModel.h"

//-----------------------------------------------------------------------------

template<size_t kSize>
class ResourceHandleArray;
namespace Adreno
{
    BOOL        FrmWriteModelToFile             ( const Model& model, const CHAR* filename );
    Model*      FrmLoadModelFromFile            ( const CHAR* filename );

    typedef     std::vector<const Mesh*>                                MeshPtrContainer;
    typedef     std::unordered_map<UINT32, MeshPtrContainer>            MeshesSortedByVertexFormat;//maps a bitfield -- one bit per GltfAttribs value -- to a set of Mesh's
    typedef     std::unordered_map<UINT32, MeshesSortedByVertexFormat>  MeshesSortedByMaterialId;
    bool        FrmLoadGLTFModelsFromTextFile   ( std::vector<Adreno::Model>*const adrenoModelsPtr, 
                                                  std::vector <std::string>* const materialFilepathsPtr, 
                                                  MeshesSortedByMaterialId* const meshesSortedByMaterialIdPtr,
                                                  const CHAR* const filename,
                                                  const UINT32 baseColorDefault_MaterialId);
    bool        FrmLoadGLTFModelsFromBinaryFile ( std::vector<Adreno::Model>*const adrenoModelsPtr, 
                                                  std::vector <std::string>* const materialFilepathsPtr, 
                                                  MeshesSortedByMaterialId* const meshesSortedByMaterialIdPtr,
                                                  const CHAR* const filename,
                                                  const UINT32 baseColorDefault_MaterialId);
    void        FrmDestroyLoadedModel           ( Adreno::Model*& model );
    
    void        FrmMeshesMergeIfSameMaterial    ( Model* const modelMergedPtr, const MeshesSortedByMaterialId& meshesSortedByMaterialIdPtr );

    void        FrmKtxFilesLoad                 (ResourceHandleArray<1024>*const textureHandlesPtr, const std::vector<std::string>& textureFilepaths);

    BOOL        FrmWriteAnimationToFile         ( const Adreno::Animation& animation, const CHAR* filename );
    Animation*  FrmLoadAnimationFromFile        ( const CHAR* filename );
    void        FrmDestroyLoadedAnimation       ( Adreno::Animation*& animation );

    bool        tinygltf_ReadWholeFile          (std::vector<unsigned char>* const output, std::string* const error, const std::string& filename);
}

//-----------------------------------------------------------------------------

#endif