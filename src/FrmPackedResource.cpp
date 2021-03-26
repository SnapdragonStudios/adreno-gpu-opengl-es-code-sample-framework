// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmPackedResource.h"
#include "FrmFile.h"
#include "FrmStdLib.h"
#include "FrmUtils.h"
//#include "Crossplatform.h"

#if LINUX_OR_OSX && !defined(ANDROID)
#include <stdio.h>
#include <netinet/in.h>
#endif

//--------------------------------------------------------------------------------------
// Name: struct PAK_FILE_HEADER
// Desc: 
//--------------------------------------------------------------------------------------
const UINT32 PAK_FILE_SIGNATURE ( ('P'<<0) | ('A'<<8) | ('K'<<16) | (0<<24) );

struct PAK_FILE_HEADER
{
    UINT32 nFileSignature;
    UINT32 nSystemDataSize;
    UINT32 nBufferDataSize;
    UINT32 nNumResources;

    // For converting from bigendian file-read data to little endian of native-host..
    void ToHostEndian()
    {
      nFileSignature= ntohl(nFileSignature);
      nSystemDataSize= ntohl(nSystemDataSize);
      nBufferDataSize= ntohl(nBufferDataSize);
      nNumResources= ntohl(nNumResources);
    }
};


//--------------------------------------------------------------------------------------
// Name: CFrmPackedResource()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmPackedResource::CFrmPackedResource()
{
    m_pSystemData   = NULL;
    m_pBufferData   = NULL;

    m_pResourceTags = NULL;
    m_nNumResources = 0;
}


//--------------------------------------------------------------------------------------
// Name: LoadFromFile()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmPackedResource::LoadFromFile( const CHAR* strFileName )
{
    struct FRM_RESOURCE_TAG_INDICES
    {
        UINT32  strName;
        UINT32  pSystemData;
        UINT32  pBufferData;
        UINT32  pObject;
    };

    // Open the file
    FRM_FILE* pFile;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &pFile ) )
    {
        return FALSE;
    }
    // Read the file header
    PAK_FILE_HEADER header;
    FrmFile_Read( pFile, &header, sizeof(header) );

    

    if( PAK_FILE_SIGNATURE != header.nFileSignature )
    {

#if LINUX_OR_OSX && !defined(ANDROID)
       if(false==IsBigEndian())
       {
           header.ToHostEndian();
       }

       if( PAK_FILE_SIGNATURE != header.nFileSignature )
#endif
       {
            FrmFile_Close( pFile );
            return FALSE;
       }
    }



    UINT32 nSystemDataSize = header.nSystemDataSize;
    UINT32 nBufferDataSize = header.nBufferDataSize;
    m_nNumResources   = header.nNumResources;

    // Read in the data from the file
    m_pSystemData = new BYTE[nSystemDataSize];
    m_pBufferData = new BYTE[nBufferDataSize];
    FrmFile_Read( pFile, m_pSystemData, nSystemDataSize );
    FrmFile_Read( pFile, m_pBufferData, nBufferDataSize );
    FrmFile_Close( pFile );

    // Fix up the resource pointers
    FRM_RESOURCE_TAG_INDICES* pResourceTagIndices = (FRM_RESOURCE_TAG_INDICES*)m_pSystemData;
    m_pResourceTags = new FRM_RESOURCE_TAG[ m_nNumResources ];

    for( UINT32 i=0; i<m_nNumResources; i++ )
    {
        m_pResourceTags[i].strName     = (CHAR*)&m_pSystemData[ pResourceTagIndices[i].strName ];
        m_pResourceTags[i].pSystemData = (BYTE*)&m_pSystemData[ pResourceTagIndices[i].pSystemData ];
        m_pResourceTags[i].pBufferData = (BYTE*)&m_pBufferData[ pResourceTagIndices[i].pBufferData ];
        m_pResourceTags[i].pObject = NULL;
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: GetResourceTags()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmPackedResource::GetResourceTags( FRM_RESOURCE_TAG** ppResourceTags,
                                          UINT32* pnNumResources )
{
    if( ppResourceTags )
        (*ppResourceTags) = m_pResourceTags;
    if( pnNumResources )
        (*pnNumResources) = m_nNumResources;
}


//--------------------------------------------------------------------------------------
// Name: GetResource()
// Desc: 
//--------------------------------------------------------------------------------------
FRM_RESOURCE_TAG* CFrmPackedResource::GetResource( const CHAR* strResource )
{
    if( NULL == strResource )
        return NULL;

    for( UINT32 i=0; i<m_nNumResources; i++ )
    {
        if( !FrmStricmp( strResource, m_pResourceTags[i].strName ) )
        {
            return &m_pResourceTags[i];
        }
    }

    return NULL;
}


//--------------------------------------------------------------------------------------
// Name: GetData()
// Desc: 
//--------------------------------------------------------------------------------------
VOID* CFrmPackedResource::GetData( const CHAR* strResource )
{
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return NULL;

    return pResourceTag->pSystemData;
}


//--------------------------------------------------------------------------------------
// Name: ReleaseResource()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmPackedResource::ReleaseResource( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return FALSE;

    if( NULL == pResourceTag->pObject )
        return FALSE;

    CFrmResource* pResource = (CFrmResource*)pResourceTag->pObject;
    pResource->Release();

    pResourceTag->pObject = NULL;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmPackedResource::Destroy()
{
    delete[] m_pSystemData;
    delete[] m_pBufferData;
    delete[] m_pResourceTags;
    m_pSystemData = NULL;
    m_pBufferData = NULL;
    m_pResourceTags = NULL;
}

