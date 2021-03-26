// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmFile.h"
#include "FrmStdLib.h"
#include "FrmUtils.h"
#include <android_native_app_glue.h>

struct FRM_FILE
{
};

AAssetManager* g_pAssetManager = 0;
const char* g_pInternalDataPath;

//--------------------------------------------------------------------------------------
// Basic file functions - only reading is currently supported on Android
//--------------------------------------------------------------------------------------
BOOL FrmFile_Open( const CHAR* strFileName, UINT32 nDirection, FRM_FILE** ppFile )
{
    if( nDirection == FRM_FILE_READ )
    {
	    AAsset* asset = AAssetManager_open(g_pAssetManager, strFileName, AASSET_MODE_STREAMING);
	    *ppFile = (FRM_FILE*)asset;
	    //LOGI("Opened asset %s at %x", strFileName, (unsigned int)asset);
	    return asset ? TRUE : FALSE;
    }
    else if( nDirection == FRM_FILE_WRITE )
    {
        // Get the internal data path for the app
        const char *internalDataPath = g_pInternalDataPath;
        if( internalDataPath == NULL )
            return FALSE;

        // Full path
        char fullPath[1024];
        char logMsg[1024];
        FrmSprintf( fullPath, sizeof(fullPath), "%s/%s", internalDataPath, strFileName );
        FrmSprintf( logMsg, sizeof(logMsg), "Opening file for writing: %s", fullPath );
        FrmLogMessage( logMsg );
        *ppFile = (FRM_FILE*)fopen( fullPath, "wb" );
        return *ppFile ? TRUE : FALSE;
    }

    return FALSE;
}

VOID FrmFile_Close( FRM_FILE* pFile, UINT32 nDirection )
{
    if( nDirection == FRM_FILE_READ )
    {
        AAsset_close((AAsset*)pFile);
    }
    else
    {
        fclose((FILE*)pFile);
    }
}

UINT32 FrmFile_Read( FRM_FILE* pFile, VOID* ptr, UINT32 nSize )
{
	return AAsset_read((AAsset*) pFile, ptr, nSize);
}

UINT32 FrmFile_FSeek(FRM_FILE *pFile, UINT32 offset, UINT32 origin)
{
    return 0;
}
UINT32 FrmFile_Write( FRM_FILE* pFile, VOID* ptr, UINT32 nSize )
{
    return fwrite( ptr, nSize, 1, (FILE*)pFile );
}

UINT32 FrmFile_GetSize( FRM_FILE* pFile )
{
    UINT32 nSize = AAsset_getRemainingLength((AAsset*)pFile);
	LOGI("GetSize returned %d", nSize);
    return nSize;
}
