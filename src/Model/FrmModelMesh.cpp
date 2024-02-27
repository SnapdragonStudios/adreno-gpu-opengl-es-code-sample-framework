//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

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

void Adreno::Mesh::AssertValid() const
{
	Vertices.AssertValid();
	Indices.AssertValid();
}