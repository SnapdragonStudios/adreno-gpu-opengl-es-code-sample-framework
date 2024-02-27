//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_INDEX_BUFFER_H
#define ADRENO_INDEX_BUFFER_H

#include "FrmPlatform.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct IndexBuffer
    {
        UINT32  NumIndices;
        UINT32* Indices;

         IndexBuffer();
        ~IndexBuffer();

        void Resize( UINT32 num_indices );
        UINT32 Index(const size_t indexOfIndex) const;
        
        void AssertValid() const;
    };
}

//-----------------------------------------------------------------------------

#endif