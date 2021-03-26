// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmVertexFormat.h"

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

void Adreno::VertexFormat::SetProperty( int index, const Adreno::VertexProperty& vertex_property )
{
    Properties[ index ] = vertex_property;
    UpdateOffsetsAndStride();
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
