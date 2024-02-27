//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmVertexProperty.h"

#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

Adreno::VertexProperty::VertexProperty()
: Usage()
, Type( Adreno::INVALID_PROPERTY_TYPE )
, Offset( 0 )
{

}

//-----------------------------------------------------------------------------

UINT32 Adreno::VertexProperty::Size() const
{
    UINT32 size = 0;
    
    switch( Type )
    {
        case Adreno::BYTE1_TYPE:
        case Adreno::BYTE1N_TYPE:
        case Adreno::UBYTE1_TYPE:
        case Adreno::UBYTE1N_TYPE:
        {
            size = 1;
            break;
        }

        case Adreno::BYTE2_TYPE:
        case Adreno::BYTE2N_TYPE:
        case Adreno::UBYTE2_TYPE:
        case Adreno::UBYTE2N_TYPE:
        {
            size = 2;
            break;
        }

        case Adreno::BYTE3_TYPE:
        case Adreno::BYTE3N_TYPE:
        case Adreno::UBYTE3_TYPE:
        case Adreno::UBYTE3N_TYPE:
        {
            size = 3;
            break;
        }

        case Adreno::BYTE4_TYPE:
        case Adreno::BYTE4N_TYPE:
        case Adreno::UBYTE4_TYPE:
        case Adreno::UBYTE4N_TYPE:
        {
            size = 4;
            break;
        }

        case Adreno::SHORT1_TYPE:
        case Adreno::SHORT1N_TYPE:
        case Adreno::USHORT1_TYPE:
        case Adreno::USHORT1N_TYPE:
        {
            size = 2;
            break;
        }

        case Adreno::SHORT2_TYPE:
        case Adreno::SHORT2N_TYPE:
        case Adreno::USHORT2_TYPE:
        case Adreno::USHORT2N_TYPE:
        {
            size = 4;
            break;
        }

        case Adreno::SHORT3_TYPE:
        case Adreno::SHORT3N_TYPE:
        case Adreno::USHORT3_TYPE:
        case Adreno::USHORT3N_TYPE:
        {
            size = 6;
            break;
        }

        case Adreno::SHORT4_TYPE:
        case Adreno::SHORT4N_TYPE:
        case Adreno::USHORT4_TYPE:
        case Adreno::USHORT4N_TYPE:
        {
            size = 8;
            break;
        }

        case Adreno::INT1_TYPE:
        case Adreno::INT1N_TYPE:
        case Adreno::UINT1_TYPE:
        case Adreno::UINT1N_TYPE:
        {
            size = 4;
            break;
        }

        case Adreno::INT2_TYPE:
        case Adreno::INT2N_TYPE:
        case Adreno::UINT2_TYPE:
        case Adreno::UINT2N_TYPE:
        {
            size = 8;
            break;
        }
        case Adreno::INT3_TYPE:
        case Adreno::INT3N_TYPE:
        case Adreno::UINT3_TYPE:
        case Adreno::UINT3N_TYPE:
        {
            size = 12;
            break;
        }

        case Adreno::INT4_TYPE:
        case Adreno::INT4N_TYPE:
        case Adreno::UINT4_TYPE:
        case Adreno::UINT4N_TYPE:
        {
            size = 16;
            break;
        }

        case Adreno::FLOAT1_TYPE:
        {
            size = 4;
            break;
        }

        case Adreno::FLOAT2_TYPE:
        {
            size = 8;
            break;
        }

        case Adreno::FLOAT3_TYPE:
        {
            size = 12;
            break;
        }

        case Adreno::FLOAT4_TYPE:
        {
            size = 16;
            break;
        }

        case Adreno::INVALID_PROPERTY_TYPE:
        case Adreno::NUM_VERTEX_PROPERTY_TYPES:
        case Adreno::VERTEX_PROPERTY_TYPE_MAX:
        {
            size = 0;
            break;
        }
    }

    return size;
}

//-----------------------------------------------------------------------------

UINT32 Adreno::VertexProperty::NumValues() const
{
    UINT32 num_values = 0;
    
    switch( Type )
    {
        case Adreno::BYTE1_TYPE:
        case Adreno::BYTE1N_TYPE:
        case Adreno::UBYTE1_TYPE:
        case Adreno::UBYTE1N_TYPE:
        {
            num_values = 1;
            break;
        }

        case Adreno::BYTE2_TYPE:
        case Adreno::BYTE2N_TYPE:
        case Adreno::UBYTE2_TYPE:
        case Adreno::UBYTE2N_TYPE:
        {
            num_values = 2;
            break;
        }

        case Adreno::BYTE3_TYPE:
        case Adreno::BYTE3N_TYPE:
        case Adreno::UBYTE3_TYPE:
        case Adreno::UBYTE3N_TYPE:
        {
            num_values = 3;
            break;
        }

        case Adreno::BYTE4_TYPE:
        case Adreno::BYTE4N_TYPE:
        case Adreno::UBYTE4_TYPE:
        case Adreno::UBYTE4N_TYPE:
        {
            num_values = 4;
            break;
        }

        case Adreno::SHORT1_TYPE:
        case Adreno::SHORT1N_TYPE:
        case Adreno::USHORT1_TYPE:
        case Adreno::USHORT1N_TYPE:
        {
            num_values = 1;
            break;
        }

        case Adreno::SHORT2_TYPE:
        case Adreno::SHORT2N_TYPE:
        case Adreno::USHORT2_TYPE:
        case Adreno::USHORT2N_TYPE:
        {
            num_values = 2;
            break;
        }

        case Adreno::SHORT3_TYPE:
        case Adreno::SHORT3N_TYPE:
        case Adreno::USHORT3_TYPE:
        case Adreno::USHORT3N_TYPE:
        {
            num_values = 3;
            break;
        }

        case Adreno::SHORT4_TYPE:
        case Adreno::SHORT4N_TYPE:
        case Adreno::USHORT4_TYPE:
        case Adreno::USHORT4N_TYPE:
        {
            num_values = 4;
            break;
        }

        case Adreno::INT1_TYPE:
        case Adreno::INT1N_TYPE:
        case Adreno::UINT1_TYPE:
        case Adreno::UINT1N_TYPE:
        {
            num_values = 1;
            break;
        }

        case Adreno::INT2_TYPE:
        case Adreno::INT2N_TYPE:
        case Adreno::UINT2_TYPE:
        case Adreno::UINT2N_TYPE:
        {
            num_values = 2;
            break;
        }
        case Adreno::INT3_TYPE:
        case Adreno::INT3N_TYPE:
        case Adreno::UINT3_TYPE:
        case Adreno::UINT3N_TYPE:
        {
            num_values = 3;
            break;
        }

        case Adreno::INT4_TYPE:
        case Adreno::INT4N_TYPE:
        case Adreno::UINT4_TYPE:
        case Adreno::UINT4N_TYPE:
        {
            num_values = 4;
            break;
        }

        case Adreno::FLOAT1_TYPE:
        {
            num_values = 1;
            break;
        }

        case Adreno::FLOAT2_TYPE:
        {
            num_values = 2;
            break;
        }

        case Adreno::FLOAT3_TYPE:
        {
            num_values = 3;
            break;
        }

        case Adreno::FLOAT4_TYPE:
        {
            num_values = 4;
            break;
        }

        case Adreno::INVALID_PROPERTY_TYPE:
        case Adreno::NUM_VERTEX_PROPERTY_TYPES:
        case Adreno::VERTEX_PROPERTY_TYPE_MAX:
        {
            num_values = 0;
            break;
        }
    }

    return num_values;
}

//-----------------------------------------------------------------------------

bool Adreno::VertexProperty::IsNormalized() const
{
    bool is_normalized = false;
    
    switch( Type )
    {
        case Adreno::BYTE1_TYPE:
        case Adreno::BYTE2_TYPE:
        case Adreno::BYTE3_TYPE:
        case Adreno::BYTE4_TYPE:
        case Adreno::UBYTE1_TYPE:
        case Adreno::UBYTE2_TYPE:
        case Adreno::UBYTE3_TYPE:
        case Adreno::UBYTE4_TYPE:
        case Adreno::SHORT1_TYPE:
        case Adreno::SHORT2_TYPE:
        case Adreno::SHORT3_TYPE:
        case Adreno::SHORT4_TYPE:
        case Adreno::USHORT1_TYPE:
        case Adreno::USHORT2_TYPE:
        case Adreno::USHORT3_TYPE:
        case Adreno::USHORT4_TYPE:
        case Adreno::INT1_TYPE:
        case Adreno::INT2_TYPE:
        case Adreno::INT3_TYPE:
        case Adreno::INT4_TYPE:
        case Adreno::UINT1_TYPE:
        case Adreno::UINT2_TYPE:
        case Adreno::UINT3_TYPE:
        case Adreno::UINT4_TYPE:
        case Adreno::FLOAT1_TYPE:
        case Adreno::FLOAT2_TYPE:
        case Adreno::FLOAT3_TYPE:
        case Adreno::FLOAT4_TYPE:
        {
            is_normalized = false;
            break;
        }

        case Adreno::BYTE1N_TYPE:
        case Adreno::BYTE2N_TYPE:
        case Adreno::BYTE3N_TYPE:
        case Adreno::BYTE4N_TYPE:
        case Adreno::UBYTE1N_TYPE:
        case Adreno::UBYTE2N_TYPE:
        case Adreno::UBYTE3N_TYPE:
        case Adreno::UBYTE4N_TYPE:
        case Adreno::SHORT1N_TYPE:
        case Adreno::SHORT2N_TYPE:
        case Adreno::SHORT3N_TYPE:
        case Adreno::SHORT4N_TYPE:
        case Adreno::USHORT1N_TYPE:
        case Adreno::USHORT2N_TYPE:
        case Adreno::USHORT3N_TYPE:
        case Adreno::USHORT4N_TYPE:
        case Adreno::INT1N_TYPE:
        case Adreno::INT2N_TYPE:
        case Adreno::INT3N_TYPE:
        case Adreno::INT4N_TYPE:
        case Adreno::UINT1N_TYPE:
        case Adreno::UINT2N_TYPE:
        case Adreno::UINT3N_TYPE:
        case Adreno::UINT4N_TYPE:
        {
            is_normalized = true;
            break;
        }

        case Adreno::INVALID_PROPERTY_TYPE:
        case Adreno::NUM_VERTEX_PROPERTY_TYPES:
        case Adreno::VERTEX_PROPERTY_TYPE_MAX:
        {
            ADRENO_ASSERT(false, __FILE__, __LINE__);
            is_normalized = false;
            break;
        }
    }

    return is_normalized;
}

bool Adreno::VertexProperty::Valid() const
{
    ADRENO_ASSERT(Type >= Adreno::VertexPropertyType::INVALID_PROPERTY_TYPE, __FILE__, __LINE__);
    ADRENO_ASSERT(Type < Adreno::VertexPropertyType::NUM_VERTEX_PROPERTY_TYPES, __FILE__, __LINE__);

    return Type != Adreno::VertexPropertyType::INVALID_PROPERTY_TYPE;
}
