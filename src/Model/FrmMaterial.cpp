// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmMaterial.h"

//-----------------------------------------------------------------------------

Adreno::Material::Material()
: Id()
{

}

//-----------------------------------------------------------------------------

void Adreno::Material::SetName( const char* name )
{
    Id.SetName( name );
}
