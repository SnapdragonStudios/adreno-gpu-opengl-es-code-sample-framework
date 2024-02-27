//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_MESH_H
#define ADRENO_MESH_H

#include "FrmIndexBuffer.h"
#include "FrmMeshSurfaceArray.h"
#include "FrmVertexBuffer.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Mesh
    {
        VertexBuffer     Vertices;
        IndexBuffer      Indices;
        MeshSurfaceArray Surfaces;
        INT32            JointIndex;
        
        Mesh();

        void AssertValid() const;
    };
}

//-----------------------------------------------------------------------------

#endif