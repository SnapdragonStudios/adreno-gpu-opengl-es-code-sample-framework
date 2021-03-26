// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmVertexBuffer.h"

//-----------------------------------------------------------------------------

Adreno::VertexBuffer::VertexBuffer()
: Format()
, NumVerts( 0 )
, BufferSize( 0 )
, Buffer( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::VertexBuffer::~VertexBuffer()
{
    if( Buffer )
    {
        delete[] Buffer;
        Buffer = NULL;
    }
}

//-----------------------------------------------------------------------------

void Adreno::VertexBuffer::IntializeBuffer( UINT32 num_verts )
{
    if( Buffer )
    {
        delete[] Buffer;
        Buffer = NULL;
    }

    NumVerts = num_verts;
    
    INT32 vertex_stride = Format.Stride;
    BufferSize          = num_verts * vertex_stride;

    if( BufferSize > 0 )
    {
        Buffer = new UINT8[ BufferSize ];
    }
}
