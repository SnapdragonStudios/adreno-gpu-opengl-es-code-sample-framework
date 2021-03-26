// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_TRANSFORM_H
#define ADRENO_TRANSFORM_H

#include "FrmMath.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Transform
    {
        FRMVECTOR3 Position;
        FRMVECTOR4 Rotation;
        FRMVECTOR3 Scale;
        
        Transform()
        : Position( 0.0f, 0.0f, 0.0f )
        , Rotation( 0.0f, 0.0f, 0.0f, 1.0f )
        , Scale( 1.0f, 1.0f, 1.0f )
        {
        
        }
    };
}

//-----------------------------------------------------------------------------

#endif