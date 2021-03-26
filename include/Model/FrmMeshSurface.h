// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_MESH_SURFACE_H
#define ADRENO_MESH_SURFACE_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct MeshSurface
    {
        UINT32 StartIndex;
        UINT32 NumTriangles;
        UINT32 MaterialId;
    };
}

//-----------------------------------------------------------------------------

#endif