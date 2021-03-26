// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_MESH_SURFACE_ARRAY_H
#define ADRENO_MESH_SURFACE_ARRAY_H

#include "FrmPlatform.h"
#include "FrmMeshSurface.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct MeshSurfaceArray
    {
        UINT32       NumSurfaces;
        MeshSurface* Surfaces;

         MeshSurfaceArray();
        ~MeshSurfaceArray();

        void Resize( UINT32 num_surfaces );
    };
}

//-----------------------------------------------------------------------------

#endif