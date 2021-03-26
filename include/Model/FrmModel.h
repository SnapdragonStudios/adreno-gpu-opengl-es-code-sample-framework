// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_MODEL_H
#define ADRENO_MODEL_H

#include "FrmJoint.h"
#include "FrmMaterial.h"
#include "FrmModelMesh.h"
#include "FrmPlatform.h"

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