//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef ADRENO_VERTEX_FORMAT_H
#define ADRENO_VERTEX_FORMAT_H

#include "FrmPlatform.h"
#include "FrmVertexProperty.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct VertexFormat
    {
        INT32           Stride;
        INT32           NumProperties;
        VertexProperty* Properties;

         VertexFormat();
        ~VertexFormat();

        VertexFormat& operator=(const VertexFormat& rhs);

        void                    ResizeProperties        ( int num_properties );
        void                    SetProperty             ( int index, const VertexProperty& vertex_property );
        const VertexProperty*   GetProperty             ( int index ) const;
        void                    UpdateOffsetsAndStride  ();

        void                    AssertIndexValid        ( const int index ) const;
        void                    AssertValid             () const;
    };
}

//-----------------------------------------------------------------------------

#endif