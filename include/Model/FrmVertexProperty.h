// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_VERTEX_PROPERTY_H
#define ADRENO_VERTEX_PROPERTY_H

#include "FrmNamedId.h"
#include "FrmPlatform.h"
#include "FrmVertexPropertyType.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexProperty
    {
        NamedId            Usage;
        VertexPropertyType Type;
        UINT32             Offset;
        
        VertexProperty();

        UINT32  Size         () const;
        UINT32  NumValues    () const;
        bool    IsNormalized () const;
    };
}

//-----------------------------------------------------------------------------

#endif