// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmUtils.h"
#include "FrmFile.h"
//#include "Crossplatform.h"
#include "GLES2/gl2.h"

#if (LINUX_OR_OSX || defined(__linux__)) && !defined(ANDROID)
#include <stdint.h>


// Programmatically detect if the system is big endian..
bool IsBigEndian(void)
{
    union
    {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1; 
}
#endif
//--------------------------------------------------------------------------------------
// Name: FrmLoadFile()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmLoadFile( const CHAR* strFileName, VOID** ppData, UINT32* pnSize )
{
    FRM_FILE* pFile;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &pFile ) )
        return FALSE;

    UINT32 nSize = FrmFile_GetSize( pFile );

    if( 0 == nSize )
    {
        FrmFile_Close( pFile );
        return FALSE;
    }

    if( ppData )
    {
        BYTE* pData = new BYTE[nSize+1];
        FrmFile_Read( pFile, pData, nSize );
        pData[nSize] = 0;

        (*ppData) = pData;
    }

    if( pnSize )
        (*pnSize) = nSize;

    FrmFile_Close( pFile );
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmUnloadFile()
// Desc: 
//--------------------------------------------------------------------------------------
VOID FrmUnloadFile( VOID* pData )
{
    delete[] pData;
}


BYTE* FrmUtils_LoadTGA( const CHAR* strFileName, UINT32* pWidth, UINT32* pHeight, UINT32* nFormat )
{
    struct TARGA_HEADER
    {
        BYTE   IDLength, ColormapType, ImageType;
        BYTE   ColormapSpecification[5];
        UINT16 XOrigin, YOrigin;
        UINT16 ImageWidth, ImageHeight;
        BYTE   PixelDepth;
        BYTE   ImageDescriptor;
    };
    
    static TARGA_HEADER header;
    
    // Read the TGA file
    FRM_FILE *fp;
    FrmFile_Open(strFileName, FRM_FILE_READ, &fp);
    if( NULL == fp )
        return NULL;

    FrmFile_Read(fp, &header, sizeof(header));

	// this Targa loader only supports uncompressed true-color images
	if( header.ImageType != 2 )
	{
		FrmFile_Close(fp);
		return NULL;
	}

    UINT32 nPixelSize = header.PixelDepth / 8;
    (*pWidth)  = header.ImageWidth;
    (*pHeight) = header.ImageHeight;
    (*nFormat) = nPixelSize == 3 ? GL_RGB : GL_RGBA;

	// skip the ID
    FrmFile_FSeek(fp, header.IDLength, FRM_FILE_SEEK_CUR);

    // load the image data
    UINT32 nSize = nPixelSize * header.ImageWidth * header.ImageHeight;
    BYTE* pBits = new BYTE[ nSize ];
    FrmFile_Read(fp, pBits, nSize);
    FrmFile_Close(fp);

    // Convert the image from BGRA to RGBA
    BYTE* p = pBits;
    for( UINT32 y=0; y<header.ImageHeight; y++ )
    {
        for( UINT32 x=0; x<header.ImageWidth; x++ )
        {
            BYTE temp = p[2]; p[2] = p[0]; p[0] = temp;
            p += nPixelSize;
        }
    }

    return pBits;
}


//--------------------------------------------------------------------------------------
// Name: LoadATC()
// Desc: Helper function to load an ATC compressed image file
//--------------------------------------------------------------------------------------
UINT8* FrmUtils_LoadATC( const CHAR* strFileName, UINT32* pWidth, UINT32* pHeight, 
                  UINT32* nFormat, UINT32* nSize )
{
    struct ATC_HEADER 
    {
        UINT32	signature;
        UINT32	width;
        UINT32	height;
        UINT32	flags;
        UINT32	dataOffset;  // From start of header/file
    } header;
   
    // Read the file
    FRM_FILE *fp;
    FrmFile_Open(strFileName,FRM_FILE_READ,&fp);
    if( NULL == fp )
        return NULL;

    FrmFile_Read(fp, &header, sizeof(header));
    UINT32 nTotalBlocks, nBytesPerBlock, nHasAlpha;

    nTotalBlocks = ((header.width + 3) >> 2) * ((header.height + 3) >> 2);
    nHasAlpha = header.flags & ATC_RGBA;
    nBytesPerBlock = (nHasAlpha) ? 16 : 8;

    (*nSize)   = nTotalBlocks * nBytesPerBlock;
    (*pWidth)  = header.width;
    (*pHeight) = header.height;
    (*nFormat) = (header.flags & ATC_RGB )? GL_ATC_RGB_AMD : GL_ATC_RGBA_AMD;

    UINT8* pBits8 = new UINT8[ *nSize ];

    // Read the encoded image 
    FrmFile_FSeek(fp, header.dataOffset, FRM_FILE_SEEK_SET);
    FrmFile_Read(fp,pBits8, *nSize );
    FrmFile_Close(fp);

    return pBits8;
}




//--------------------------------------------------------------------------------------
// Name: FrmSaveImageAsTGA()
// Desc: Save a 32-bit RGBA image as a TGA file.
//--------------------------------------------------------------------------------------
BOOL FrmSaveImageAsTGA( const CHAR* strFileName, INT16 nWidth, INT16 nHeight,
                        UINT32* pBits32 )
{
    typedef struct _TARGA_HEADER
    {
        BYTE   IDLength;
        BYTE   ColormapType;
        BYTE   ImageType;
        BYTE   ColormapSpecification[5];
        UINT16 XOrigin;
        UINT16 YOrigin;
        UINT16 ImageWidth;
        UINT16 ImageHeight;
        BYTE   PixelDepth;
        BYTE   ImageDescriptor;
    } TARGA_HEADER;
    
    // Create the TGA file
    FRM_FILE* pFile;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_WRITE, &pFile ) )
        return FALSE;

    // Write out the TGA header
    TARGA_HEADER header;
    FrmMemset( &header, 0, sizeof(header) );
    header.ImageType   = 2;
    header.ImageWidth  = nWidth;
    header.ImageHeight = nHeight;
    header.PixelDepth  = 32;
    FrmFile_Write( pFile, &header, sizeof(header) );

    // Write out the pixels
    FrmFile_Write( pFile, pBits32, sizeof(UINT32)*nWidth*nHeight );

    // Close the file and return
    FrmFile_Close( pFile, FRM_FILE_WRITE );
    return TRUE;
}

