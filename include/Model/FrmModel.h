//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_MODEL_H
#define ADRENO_MODEL_H

#include "FrmJoint.h"
#include "FrmMaterial.h"
#include "FrmModelMesh.h"
#include "FrmPlatform.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Model
    {
        INT32     NumMaterials;
        Material* Materials;

        INT32     NumMeshes;
        Mesh*     Meshes;
        
        INT32     NumJoints;
        Joint*    Joints;

         Model();
        ~Model();

        inline const Mesh* MeshGet(const size_t i) const
        {
            ADRENO_ASSERT(i < NumMeshes, __FILE__, __LINE__);
            return &Meshes[i];
        }
        inline Mesh* MeshGet(const size_t i)
        {
            return const_cast<Mesh*>(static_cast<const Model*>(this)->MeshGet(i));
        }

        inline const Material* MaterialGet(const size_t i) const
        {
            ADRENO_ASSERT(NumMaterials > 0, __FILE__, __LINE__);
            ADRENO_ASSERT(i < NumMaterials, __FILE__, __LINE__);
            return &Materials[i];
        }
        inline Material* MaterialGet(const size_t i)
        {
            return const_cast<Material*>(static_cast<const Model*>(this)->MaterialGet(i));
        }

        void    ResizeMaterials ( int num_materials );
        void    ResizeMeshes    ( int num_meshes );
        void    ResizeJoints    ( int num_joints );

        private:
            void DeleteMaterials();
            void DeleteMeshes();
            void DeleteJoints();
    };
}

//-----------------------------------------------------------------------------

#endif