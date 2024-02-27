//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmVertexFormat.h"
#include "FrmStdLib.h"
#include <string.h>

//-----------------------------------------------------------------------------

Adreno::VertexFormat::VertexFormat()
: Stride( 0 )
, NumProperties( 0 )
, Properties( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::VertexFormat::~VertexFormat()
{
    if( Properties )
    {
        delete[] Properties;
        Properties = NULL;
    }
}

//-----------------------------------------------------------------------------

Adreno::VertexFormat& Adreno::VertexFormat::operator=(const VertexFormat& rhs)
{
    Stride = rhs.Stride;

    ResizeProperties(rhs.NumProperties);
    memcpy(Properties, rhs.Properties, sizeof(Properties[0]) * NumProperties);

    return *this;
}

//-----------------------------------------------------------------------------

void Adreno::VertexFormat::ResizeProperties( int num_properties )
{
    if( Properties )
    {
        delete[] Properties;
        Properties = NULL;
    }

    NumProperties = num_properties;

    if( num_properties > 0 )
    {
        Properties = new Adreno::VertexProperty[ num_properties ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::VertexFormat::AssertIndexValid(const int index) const
{
    ADRENO_ASSERT(index >= 0, __FILE__, __LINE__);
    ADRENO_ASSERT(index < NumProperties, __FILE__, __LINE__);
    ADRENO_ASSERT(Properties, __FILE__, __LINE__);
}

//-----------------------------------------------------------------------------

void Adreno::VertexFormat::SetProperty( int index, const Adreno::VertexProperty& vertex_property )
{
    AssertIndexValid(index);
    
    Properties[ index ] = vertex_property;
    UpdateOffsetsAndStride();
}

//-----------------------------------------------------------------------------

const Adreno::VertexProperty* Adreno::VertexFormat::GetProperty( int index ) const
{
    AssertIndexValid(index);
    return &Properties[ index ];
}

//-----------------------------------------------------------------------------

void Adreno::VertexFormat::UpdateOffsetsAndStride()
{
    Stride = 0;

    for( int i = 0; i < NumProperties; ++i )
    {
        Adreno::VertexProperty& vertex_property = Properties[ i ];
        vertex_property.Offset = Stride;

        UINT32 property_size = vertex_property.Size();
        Stride += property_size;
    }
}

//-----------------------------------------------------------------------------

void Adreno::VertexFormat::AssertValid() const
{
    ADRENO_ASSERT(Stride > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(NumProperties > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(Properties, __FILE__, __LINE__);

    for (INT32 i = 0; i < NumProperties; ++i)
    {
        if (Properties[i].Valid())
        {
            //Usage string should not be duplicated across a single vertex format
            ADRENO_ASSERT(CStringNotEmpty(&Properties[i].Usage.Name[0]), __FILE__, __LINE__);
            for (INT32 j = 0; j < NumProperties; ++j)
            {
                if (i == j || !Properties[j].Valid())
                {
                    continue;
                }
                ADRENO_ASSERT(strncmp(&Properties[i].Usage.Name[0], &Properties[j].Usage.Name[0], Adreno::NamedId::MAX_NAME_LENGTH) != 0, __FILE__, __LINE__);
            }
        }
    }
}