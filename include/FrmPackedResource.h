// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_PACKED_RESOURCE_H_
#define _FRM_PACKED_RESOURCE_H_

#include "FrmPlatform.h"
#include "FrmResource.h"


//--------------------------------------------------------------------------------------
// Name: struct FRM_RESOURCE_TAG
// Desc: Structure to hold data about a resource in a packed resource file
//--------------------------------------------------------------------------------------
struct FRM_RESOURCE_TAG
{
    CHAR*  strName;
    BYTE*  pSystemData;
    BYTE*  pBufferData;
    BYTE*  pObject;
};


//--------------------------------------------------------------------------------------
// Name: class CFrmPackedResource
// Desc: Loads and extracts resources from a packed resource file
//--------------------------------------------------------------------------------------
class CFrmPackedResource
{
public:
    CFrmPackedResource();
    ~CFrmPackedResource() { Destroy(); }

    BOOL              LoadFromFile( const CHAR* strFileName );

    VOID              GetResourceTags( FRM_RESOURCE_TAG** ppResourceTags, UINT32* pnNumResources );

    FRM_RESOURCE_TAG* GetResource( const CHAR* strResource );
    VOID*             GetData( const CHAR* strResource );

    BOOL              ReleaseResource( const CHAR* strResource );
    
    VOID              Destroy();

protected:
    BYTE*             m_pSystemData;
    BYTE*             m_pBufferData;

    FRM_RESOURCE_TAG* m_pResourceTags;
    UINT32            m_nNumResources;
};


#endif // _FRM_PACKED_RESOURCE_H_
