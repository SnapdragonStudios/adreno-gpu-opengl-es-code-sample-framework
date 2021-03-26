// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_VERTEX_PROPERTY_TYPE_H
#define ADRENO_VERTEX_PROPERTY_TYPE_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    enum VertexPropertyType
    {
        INVALID_PROPERTY_TYPE,

        BYTE1_TYPE,
        BYTE2_TYPE,
        BYTE3_TYPE,
        BYTE4_TYPE,

        BYTE1N_TYPE,
        BYTE2N_TYPE,
        BYTE3N_TYPE,
        BYTE4N_TYPE,

        UBYTE1_TYPE,
        UBYTE2_TYPE,
        UBYTE3_TYPE,
        UBYTE4_TYPE,

        UBYTE1N_TYPE,
        UBYTE2N_TYPE,
        UBYTE3N_TYPE,
        UBYTE4N_TYPE,

        SHORT1_TYPE,
        SHORT2_TYPE,
        SHORT3_TYPE,
        SHORT4_TYPE,

        SHORT1N_TYPE,
        SHORT2N_TYPE,
        SHORT3N_TYPE,
        SHORT4N_TYPE,

        USHORT1_TYPE,
        USHORT2_TYPE,
        USHORT3_TYPE,
        USHORT4_TYPE,

        USHORT1N_TYPE,
        USHORT2N_TYPE,
        USHORT3N_TYPE,
        USHORT4N_TYPE,

        INT1_TYPE,
        INT2_TYPE,
        INT3_TYPE,
        INT4_TYPE,

        INT1N_TYPE,
        INT2N_TYPE,
        INT3N_TYPE,
        INT4N_TYPE,

        UINT1_TYPE,
        UINT2_TYPE,
        UINT3_TYPE,
        UINT4_TYPE,

        UINT1N_TYPE,
        UINT2N_TYPE,
        UINT3N_TYPE,
        UINT4N_TYPE,

        FLOAT1_TYPE,
        FLOAT2_TYPE,
        FLOAT3_TYPE,
        FLOAT4_TYPE,
        
        NUM_VERTEX_PROPERTY_TYPES,
        VERTEX_PROPERTY_TYPE_MAX = 0xffffffff // Force this to be a 32-bit value
    };
}

//-----------------------------------------------------------------------------

#endif