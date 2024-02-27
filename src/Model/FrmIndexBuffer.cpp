//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmIndexBuffer.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

Adreno::IndexBuffer::IndexBuffer()
: NumIndices( 0 )
, Indices( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::IndexBuffer::~IndexBuffer()
{
    if( Indices )
    {
        delete[] Indices;
        Indices = NULL;
   }
}

//-----------------------------------------------------------------------------

void Adreno::IndexBuffer::Resize( UINT32 num_indices )
{
    if( Indices )
    {
        delete[] Indices;
        Indices = NULL;
   }

    NumIndices = num_indices;

    if( num_indices > 0 )
    {
        Indices = new UINT32[ num_indices ];
    }
}

//-----------------------------------------------------------------------------
UINT32 Adreno::IndexBuffer::Index(const size_t indexOfIndex) const
{
    ADRENO_ASSERT(Indices, __FILE__, __LINE__);
    ADRENO_ASSERT(indexOfIndex < NumIndices, __FILE__, __LINE__);

    return Indices[indexOfIndex];
}

//-----------------------------------------------------------------------------
void Adreno::IndexBuffer::AssertValid() const
{
    ADRENO_ASSERT(NumIndices > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(Indices, __FILE__, __LINE__);
}