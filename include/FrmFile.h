// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_FILE_H_
#define _FRM_FILE_H_

#include "FrmPlatform.h"


// Platform-agnostic file handle, defined in platform-specific code
struct FRM_FILE;


//--------------------------------------------------------------------------------------
// FrmFile_Open flags
//--------------------------------------------------------------------------------------
const UINT32 FRM_FILE_READ      = 0;
const UINT32 FRM_FILE_WRITE     = 1;
const UINT32 FRM_FILE_READWRITE = 2;

const UINT32 FRM_FILE_SEEK_SET = 0;
const UINT32 FRM_FILE_SEEK_CUR = 1;
const UINT32 FRM_FILE_SEEK_END = 2;


//--------------------------------------------------------------------------------------
// Basic file functions
//--------------------------------------------------------------------------------------
#ifndef UNDER_CE
    BOOL   FrmFile_Open( const CHAR* strFileName, UINT32 nDirection, FRM_FILE** ppFile );
    VOID   FrmFile_Close( FRM_FILE* pFile, UINT32 nDirection = FRM_FILE_READ );
    UINT32 FrmFile_Read( FRM_FILE* pFile, VOID* ptr, UINT32 size );
    UINT32 FrmFile_FSeek(FRM_FILE *, UINT32 , UINT32);
    UINT32 FrmFile_Write( FRM_FILE* pFile, VOID* ptr, UINT32 size );
    UINT32 FrmFile_GetSize( FRM_FILE* pFile );
#else
    BOOL   FrmFile_Open( const CHAR* strFileName, UINT32 nDirection, FILE** ppFile );
    VOID   FrmFile_Close( FILE* pFile, UINT32 nDirection = FRM_FILE_READ );
    UINT32 FrmFile_Read( FILE* pFile, VOID* ptr, UINT32 size );
    UINT32 FrmFile_FSeek(FRM_FILE *, UINT32 , UINT32 );
    UINT32 FrmFile_Write( FILE* pFile, VOID* ptr, UINT32 size );
    UINT32 FrmFile_GetSize( FILE* pFile );
#endif // !UNDER_CE


#endif // _FRM_FILE_H_
