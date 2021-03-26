// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

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

        void    ResizeProperties        ( int num_properties );
        void    SetProperty             ( int index, const VertexProperty& vertex_property );
        void    UpdateOffsetsAndStride  ();
    };
}

//-----------------------------------------------------------------------------

#endif