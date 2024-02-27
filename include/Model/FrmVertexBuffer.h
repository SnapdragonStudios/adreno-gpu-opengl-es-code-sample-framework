//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_VERTEX_BUFFER_H
#define ADRENO_VERTEX_BUFFER_H

#include "FrmPlatform.h"
#include "FrmVertexFormat.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexBuffer
    {
        VertexFormat Format;
        UINT32       NumVerts;
        UINT32       BufferSize;
        UINT8*       Buffer;
        
         VertexBuffer();
        ~VertexBuffer();

        void InitializeBuffer( UINT32 num_verts );
        void AssertValid() const;
    };
}

//-----------------------------------------------------------------------------

#endif