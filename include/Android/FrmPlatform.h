//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _FRM_PLATFORM_H_
#define _FRM_PLATFORM_H_

//--------------------------------------------------------------------------------------
// Platform
//--------------------------------------------------------------------------------------
#include <jni.h>
#include <errno.h>
#include <stdio.h>
#include <android/sensor.h>
#include <android/log.h>
#include <android_native_app_glue.h>

#if (_OGLES3)
//#include <GLES3/gl3.h>
#include <OpenGLES/FrmGLES3.h>
#else
#include <GLES2/gl2.h>
#endif

// Linux - Network
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

// Linux - Threads
#include <pthread.h>

// Linux - semaphores
#include <semaphore.h>

#include <assert.h>

#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "adreno-sdk-framework", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "adreno-sdk-framework", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "adreno-sdk-framework", __VA_ARGS__))

#define _CRT_STRINGIZE(x) #x
#define PLATFORM ANDROID

static void CheckGlError(const char* op)
{
	for (GLint error = glGetError(); error; error = glGetError())
	{
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}

//--------------------------------------------------------------------------------------
// Common types
//--------------------------------------------------------------------------------------
typedef unsigned char      BYTE;

typedef char               CHAR;
typedef char               CHAR8;
typedef unsigned char      UCHAR;
typedef unsigned char      UCHAR8;

typedef wchar_t            WCHAR;
typedef unsigned short     UCHAR16;

typedef signed char        INT8;
typedef signed short       INT16;
typedef signed int         INT32;
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;

typedef float              FLOAT;
typedef float              FLOAT32;
typedef double             FLOAT64;

typedef int                BOOL;

typedef unsigned short     WORD;
typedef unsigned long      DWORD;

#ifndef VOID
    #define VOID void
#endif

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

#ifndef NULL
    #define NULL 0
#endif

const UINT32 NATIVE_APP_WIDTH  = 480;
const UINT32 NATIVE_APP_HEIGHT = 765;

#define SAFE_DELETE(p)              if ((p) != NULL) delete (p); (p) = NULL;
#define SAFE_FREE(p)                if ((p) != NULL) free(p); (p) = NULL;

#ifdef ANDROID_CL
VOID FrmAndroidUpdateImage( int viewID, const CHAR* path );
#endif // ANDROID_CL

#endif // _FRM_PLATFORM_H_
