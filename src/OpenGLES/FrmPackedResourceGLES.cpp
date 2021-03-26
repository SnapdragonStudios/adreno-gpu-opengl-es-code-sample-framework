// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "OpenGLES/FrmPackedResourceGLES.h"
#include "FrmFile.h"
#include "FrmStdLib.h"
#include "FrmUtils.h"

#include<stdio.h>


#ifdef ANDROID
    #define ATTRIBUTE_PACKED __attribute__ ((packed))
#else
    #define ATTRIBUTE_PACKED
#endif

struct FRAMEWORK_TEXTURE_HEADER
{
    UINT32 nWidth;      
    UINT32 nHeight;     
    UINT32 nNumLevels;  
    UINT32 nPixelFormat;
    UINT32 nPixelType;  
    UINT32 nPitch;      
    UINT32 nBaseSize;   
    UINT32 nMipChainSize;
    UINT32 nMinMipSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;  
} ATTRIBUTE_PACKED;

struct FRAMEWORK_TEXTURE3D_HEADER 
{
    UINT32 nWidth;
    UINT32 nHeight;
    UINT32 nDepth;
    UINT32 nNumLevels;
	UINT32 nPixelFormat;
	UINT32 nPixelType;
    UINT32 nRowPitch;
    UINT32 nSlicePitch;
    UINT32 nBaseSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;
} ATTRIBUTE_PACKED;

struct FRAMEWORK_CUBEMAP_HEADER 
{
    UINT32 nWidth;
    UINT32 nHeight;
    UINT32 nNumLevels;
    UINT32 nPixelFormat;
    UINT32 nPixelType;
    UINT32 nPitch;
    UINT32 nBaseSize;
    UINT32 nMipChainSize;
    UINT32 nMinMipSize;
    UINT32 nMagFilter;
    UINT32 nMinFilter;
    UINT32 nWrapMode;
} ATTRIBUTE_PACKED;


//--------------------------------------------------------------------------------------
// Name: GetTexture()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmTexture* CFrmPackedResourceGLES::GetTexture( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
    {
        return NULL;
    }

    // If the texture already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmResource* pTexture = (CFrmResource*)pResourceTag->pObject;
        pTexture->AddRef();
        return (CFrmTexture*)pTexture;
    }

    // Otherwise, create the texture using it's data stored in the packed resource
    FRAMEWORK_TEXTURE_HEADER *pTextureHeader = (FRAMEWORK_TEXTURE_HEADER*)pResourceTag->pSystemData;
    BYTE* pTextureImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;

    // Create the texture
    CFrmTexture* pTexture = NULL;
    if( FALSE == FrmCreateTexture( pTextureHeader->nWidth, pTextureHeader->nHeight,
                                   nNumLevels, nPixelFormat, nPixelType, 
                                   nBorder, pTextureImageData,
                                   pTextureHeader->nBaseSize, &pTexture ) )
    {
        return NULL;
    }
    // Set the texture filter and wrap modes
    glBindTexture( GL_TEXTURE_2D, pTexture->m_hTextureHandle );
    if( pTextureHeader->nMagFilter )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, pTextureHeader->nMagFilter );
    if( pTextureHeader->nMinFilter )
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, pTextureHeader->nMinFilter );
    if( pTextureHeader->nWrapMode )
    {
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pTextureHeader->nWrapMode );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pTextureHeader->nWrapMode );
    }

    // Return it
    pResourceTag->pObject = (BYTE*)pTexture;
    return pTexture;
}


//--------------------------------------------------------------------------------------
// Name: GetTextureHandle()
// Desc: 
//--------------------------------------------------------------------------------------
INT32 CFrmPackedResourceGLES::GetTextureHandle( const CHAR* strResource )
{
    CFrmTexture* pTexture = GetTexture( strResource );
    if( NULL == pTexture )
        return -1;

    return pTexture->m_hTextureHandle;
}


//--------------------------------------------------------------------------------------
// Name: GetTexture3D()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmTexture3D* CFrmPackedResourceGLES::GetTexture3D( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
    {
        return NULL;
    }

    // If the texture already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmResource* pTexture = (CFrmResource*)pResourceTag->pObject;
        pTexture->AddRef();
        return (CFrmTexture3D*)pTexture;
    }

    // Otherwise, create the texture using it's data stored in the packed resource

    FRAMEWORK_TEXTURE3D_HEADER* pTextureHeader = (FRAMEWORK_TEXTURE3D_HEADER*)pResourceTag->pSystemData;
    BYTE* pTextureImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;

    // Create the texture
    CFrmTexture3D* pTexture;
    if( FALSE == FrmCreateTexture3D( pTextureHeader->nWidth, pTextureHeader->nHeight, pTextureHeader->nDepth,
                                     nNumLevels, nPixelFormat, nPixelType, 
									 nBorder, pTextureImageData,
                                     pTextureHeader->nBaseSize * pTextureHeader->nDepth, &pTexture ) )
    {
        return NULL;
    }

    // Set the texture filter and wrap modes
    glBindTexture( GL_TEXTURE_3D_OES, pTexture->m_hTextureHandle );
    if( pTextureHeader->nMagFilter )
        glTexParameteri( GL_TEXTURE_3D_OES, GL_TEXTURE_MAG_FILTER, pTextureHeader->nMagFilter );
    if( pTextureHeader->nMinFilter )
        glTexParameteri( GL_TEXTURE_3D_OES, GL_TEXTURE_MIN_FILTER, pTextureHeader->nMinFilter );
    if( pTextureHeader->nWrapMode )
    {
        glTexParameteri( GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_S, pTextureHeader->nWrapMode );
        glTexParameteri( GL_TEXTURE_3D_OES, GL_TEXTURE_WRAP_T, pTextureHeader->nWrapMode );
    }

    // Return it
    pResourceTag->pObject = (BYTE*)pTexture;
    return pTexture;
}


//--------------------------------------------------------------------------------------
// Name: GetTexture3DHandle()
// Desc: 
//--------------------------------------------------------------------------------------
INT32 CFrmPackedResourceGLES::GetTexture3DHandle( const CHAR* strResource )
{
    CFrmTexture3D* pTexture3D = GetTexture3D( strResource );
    if( NULL == pTexture3D )
        return -1;

    return pTexture3D->m_hTextureHandle;
}


//--------------------------------------------------------------------------------------
// Name: GetCubeMap()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmCubeMap* CFrmPackedResourceGLES::GetCubeMap( const CHAR* strResource )
{
    // Find the requested resource tag
    FRM_RESOURCE_TAG* pResourceTag = GetResource( strResource );
    if( NULL == pResourceTag )
        return NULL;

    // If the cubemap already exists, return a reference to it
    if( pResourceTag->pObject )
    {
        CFrmCubeMap* pCubeMap = (CFrmCubeMap*)pResourceTag->pObject;
        pCubeMap->AddRef();
        return pCubeMap;
    }

    // Otherwise, create the cubemap using it's data stored in the packed resource

    FRAMEWORK_CUBEMAP_HEADER* pTextureHeader = (FRAMEWORK_CUBEMAP_HEADER*)pResourceTag->pSystemData;
    BYTE* pCubeMapImageData = pResourceTag->pBufferData;

    UINT32 nNumLevels   = pTextureHeader->nNumLevels;
    UINT32 nBorder      = 0;
    UINT32 nPixelFormat = pTextureHeader->nPixelFormat;
    UINT32 nPixelType   = pTextureHeader->nPixelType;

    // Create the cubemap
    CFrmCubeMap* pCubeMap;
    if( FALSE == FrmCreateCubeMap( pTextureHeader->nWidth, pTextureHeader->nHeight,
                                   nNumLevels, nPixelFormat, nPixelType, 
                                   nBorder, pCubeMapImageData,
                                   pTextureHeader->nMipChainSize, &pCubeMap ) )
        return NULL;

    // Set the texture filter and wrap modes
    glBindTexture( GL_TEXTURE_CUBE_MAP, pCubeMap->m_hTextureHandle );
    if( pTextureHeader->nMagFilter )
        glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, pTextureHeader->nMagFilter );
    if( pTextureHeader->nMinFilter )
        glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, pTextureHeader->nMinFilter );
    if( pTextureHeader->nWrapMode )
    {
        glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, pTextureHeader->nWrapMode );
        glTexParameteri( GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, pTextureHeader->nWrapMode );
    }

    // Return it
    pResourceTag->pObject = (BYTE*)pCubeMap;
    return pCubeMap;
}


//--------------------------------------------------------------------------------------
// Name: GetCubeMapHandle()
// Desc: 
//--------------------------------------------------------------------------------------
INT32 CFrmPackedResourceGLES::GetCubeMapHandle( const CHAR* strResource )
{
    CFrmCubeMap* pCubeMap = GetCubeMap( strResource );
    if( NULL == pCubeMap )
        return -1;
    
    return pCubeMap->m_hTextureHandle;
}
