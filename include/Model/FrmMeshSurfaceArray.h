//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_MESH_SURFACE_ARRAY_H
#define ADRENO_MESH_SURFACE_ARRAY_H

#include "FrmMeshSurface.h"
#include "FrmPlatform.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct MeshSurfaceArray
    {
        UINT32       NumSurfaces;
        MeshSurface* Surfaces;

        inline const MeshSurface* SurfaceGet(const size_t i) const
        {
            ADRENO_ASSERT(i < NumSurfaces, __FILE__, __LINE__);
            return &Surfaces[i];
        }
        inline MeshSurface* SurfaceGet(const size_t i)
        {
            return const_cast<MeshSurface*>(static_cast<const MeshSurfaceArray*>(this)->SurfaceGet(i));
        }


         MeshSurfaceArray();
        ~MeshSurfaceArray();

        void Resize( UINT32 num_surfaces );
    };
}

//-----------------------------------------------------------------------------

#endif