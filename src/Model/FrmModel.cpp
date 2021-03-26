// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmModel.h"

//-----------------------------------------------------------------------------

Adreno::Model::Model()
: NumMaterials( 0 )
, Materials( NULL )
, NumMeshes( 0 )
, Meshes( NULL )
, NumJoints( 0 )
, Joints( NULL )
{

}

//-----------------------------------------------------------------------------

Adreno::Model::~Model()
{
    DeleteMaterials();
    DeleteMeshes();
    DeleteJoints();
}

//-----------------------------------------------------------------------------

void Adreno::Model::ResizeMaterials( int num_materials )
{
    DeleteMaterials();
    
    NumMaterials = num_materials;
    
    if( num_materials > 0 )
    {
        Materials = new Adreno::Material[ num_materials ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::Model::ResizeMeshes( int num_meshes )
{
    DeleteMeshes();
    
    NumMeshes = num_meshes;
    
    if( num_meshes > 0 )
    {
        Meshes = new Adreno::Mesh[ num_meshes ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::Model::ResizeJoints( int num_joints )
{
    DeleteJoints();
    
    NumJoints = num_joints;
    
    if( num_joints > 0 )
    {
        Joints = new Adreno::Joint[ num_joints ];
    }
}

//-----------------------------------------------------------------------------

void Adreno::Model::DeleteMaterials()
{
    if( Materials )
    {
        delete[] Materials;
        Materials = NULL;
    }
}

//-----------------------------------------------------------------------------

void Adreno::Model::DeleteMeshes()
{
    if( Meshes )
    {
        delete[] Meshes;
        Meshes = NULL;
    }
}

//-----------------------------------------------------------------------------

void Adreno::Model::DeleteJoints()
{
    if( Joints )
    {
        delete[] Joints;
        Joints = NULL;
    }
}
