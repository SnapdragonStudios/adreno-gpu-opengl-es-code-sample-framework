//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmVertexBuffer.h"
#include "FrmStdLib.h"

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
        //LOGI("%s:%i:delete[] Buffer=%p", __FILE__, __LINE__, Buffer);
        Buffer = NULL;
    }
}

//-----------------------------------------------------------------------------

void Adreno::VertexBuffer::InitializeBuffer( UINT32 num_verts )
{
    if( Buffer )
    {
        delete[] Buffer;
        //LOGI("%s:%i:delete[] Buffer=%p", __FILE__, __LINE__, Buffer);
        Buffer = NULL;
    }

    NumVerts = num_verts;
    
    INT32 vertex_stride = Format.Stride;
    BufferSize          = num_verts * vertex_stride;

    if( BufferSize > 0 )
    {
        Buffer = new UINT8[ BufferSize ];
        //LOGI("%s:%i:Buffer=%p, BufferSize=%u", __FILE__, __LINE__, Buffer, BufferSize);
    }
}

//-----------------------------------------------------------------------------
void Adreno::VertexBuffer::AssertValid() const
{
    Format.AssertValid();
    ADRENO_ASSERT(NumVerts > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(BufferSize > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(Buffer, __FILE__, __LINE__);
}