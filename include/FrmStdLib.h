// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_STDLIB_H_
#define _FRM_STDLIB_H_

#include "FrmPlatform.h"


// Memory allocation functions
VOID*       FrmMalloc( UINT32 nSize );
VOID*       FrmRealloc( VOID* iptr, UINT32 nSize );
VOID        FrmFree( VOID* ptr );

// Memory manipulation functions
VOID*       FrmMemcpy( VOID* s, const VOID* ct, UINT32 n );
VOID*       FrmMemmove( VOID* s, const VOID* ct, UINT32 n );
INT32       FrmMemcmp( const VOID* cs, const VOID* ct, UINT32 n );
const VOID* FrmMemchr( const VOID* cs, BYTE c, UINT32 n );
const VOID* FrmMemset( VOID* s, BYTE c, UINT32 n );

// String manipulation functions
CHAR*       FrmStrcpy( CHAR* s, const CHAR* ct );
CHAR*       FrmStrncpy( CHAR* s, const CHAR* ct, UINT32 n );
CHAR*       FrmStrcat( CHAR* s, const CHAR* ct );
CHAR*       FrmStrncat( CHAR* s, const CHAR* ct, UINT32 n );
INT32       FrmStrcmp( const CHAR* cs, const CHAR* ct );
INT32       FrmStricmp( const CHAR* cs, const CHAR* ct );
INT32       FrmStrncmp( const CHAR* cs, const CHAR* ct, UINT32 n );
const CHAR* FrmStrchr( const CHAR* cs, CHAR c );
const CHAR* FrmStrrchr( const CHAR* cs, CHAR c );
UINT32      FrmStrspn( const CHAR* cs, const CHAR* ct );
UINT32      FrmStrcspn( const CHAR* cs, const CHAR* ct );
const CHAR* FrmStrpbrk( const CHAR* cs, const CHAR* ct );
const CHAR* FrmStrstr( const CHAR* cs, const CHAR* ct );
UINT32      FrmStrlen( const CHAR* cs );
CHAR*       FrmStrerror( INT32 n );
CHAR*       FrmStrtok( CHAR* s, const CHAR* ct );
int         FrmSprintf( CHAR*, int, const CHAR*, ... );


WCHAR*      FrmWmemcpy( WCHAR* ws1, const WCHAR* ws2, UINT32 n ); 
UINT32      FrmWcslen( const WCHAR* cs );
int         FrmSwprintf( WCHAR*, int, const WCHAR*, ... );


#endif // _FRM_STDLIB_H_
