// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_RESOURCE_H_
#define _FRM_RESOURCE_H_

#include "FrmPlatform.h"


//--------------------------------------------------------------------------------------
// Name: class CFrmResource
// Desc: The base resource class that all other resources derive from
//--------------------------------------------------------------------------------------
class CFrmResource
{
public:
    CFrmResource( UINT32 nType )
    {
        m_nResourceType = nType;
        m_nRefCount     = 1;
        m_bAllocated    = TRUE;
    }

    virtual ~CFrmResource() {}

    VOID AddRef()
    { 
        m_nRefCount++; 
    }
    
    VOID Release()
    { 
        if( 0 == --m_nRefCount ) 
            if( m_bAllocated )
                delete this;
    }

	virtual VOID Bind( UINT32 nTextureUnit=0 ) = 0;

    UINT32 m_nResourceType;
    UINT32 m_nRefCount;
    BOOL   m_bAllocated;
};


#endif // _FRM_RESOURCE_H_
