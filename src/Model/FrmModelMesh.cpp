// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmModelMesh.h"
#include "FrmJoint.h"

//-----------------------------------------------------------------------------

Adreno::Mesh::Mesh()
: Vertices()
, Indices()
, Surfaces()
, JointIndex( Adreno::Joint::NULL_NODE_INDEX )
{

}
