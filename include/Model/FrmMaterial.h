// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef ADRENO_MATERIAL_H
#define ADRENO_MATERIAL_H

#include "FrmNamedId.h"

//-----------------------------------------------------------------------------

namespace Adreno
{
    struct Material
    {
        Adreno::NamedId Id;

        Material();

        void SetName( const char* name );
    };
}

//-----------------------------------------------------------------------------

#endif