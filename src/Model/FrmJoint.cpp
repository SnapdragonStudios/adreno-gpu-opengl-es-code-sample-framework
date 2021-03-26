// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmJoint.h"
#include "FrmStdLib.h"

//-----------------------------------------------------------------------------

Adreno::Joint::Joint()
: Id()
, ParentIndex( NULL_NODE_INDEX )
, Transform()
, InverseBindPose()
{

}

//-----------------------------------------------------------------------------

void Adreno::Joint::SetName( const char* name )
{
    Id.SetName( name );
}
