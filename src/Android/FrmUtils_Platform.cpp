// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmUtils.h"
#include <assert.h>
#include <ctime>
#include <android/log.h>


android_app* g_pApp = NULL;
const char* g_log = "Unsupported";
char* g_pAppName = NULL;

//--------------------------------------------------------------------------------------
// Name: FrmAssert()
// Desc: Platform-dependent assert function.
//--------------------------------------------------------------------------------------
VOID FrmAssert( INT32 nExpression )
{
    assert( nExpression );
}


//--------------------------------------------------------------------------------------
// Name: FrmAdrenoNativeActivityLog()
// Desc: Pass a log message to the AdrenoNativeActivity class so that it can
// display the string contents on the device screen.
//--------------------------------------------------------------------------------------
VOID FrmAdrenoNativeActivityLog( const CHAR* strMessage )
{
    if( g_pApp == NULL )
        return;

    // The AdrenoNativeActivity.java class has a method logMessage() that we call here
    // so that it can display the results of running the application on the screen.
    JavaVM* vm = g_pApp->activity->vm;
	JNIEnv* env;

    vm->AttachCurrentThread( &env, 0 );

    jobject me = g_pApp->activity->clazz;
	jclass acl = env->GetObjectClass( me );
    jmethodID logMessage = env->GetMethodID(acl, "logMessage", "(Ljava/lang/String;)V");

    jstring jstr = env->NewStringUTF( strMessage );
    env->CallVoidMethod(me, logMessage, jstr);

    env->DeleteLocalRef(jstr);

    vm->DetachCurrentThread();
}

//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
VOID FrmLogMessage( const CHAR* strPrefix, const CHAR* strMessage,
                    const CHAR* strPostFix )
{
    //typedef enum android_LogPriority {
    //            ANDROID_LOG_UNKNOWN = 0,
    //            ANDROID_LOG_DEFAULT,    /* only for SetMinPriority() */
    //            ANDROID_LOG_VERBOSE,
    //            ANDROID_LOG_DEBUG,
    //            ANDROID_LOG_INFO,
    //            ANDROID_LOG_WARN,
    //            ANDROID_LOG_ERROR,
    //            ANDROID_LOG_FATAL,
    //            ANDROID_LOG_SILENT,     /* only for SetMinPriority(); must be last */
    //            } android_LogPriority;
#ifdef ANDROID_CL
    const CHAR* pLogTag = "OpenCL 1.1 Embedded Samples";
    if( g_pAppName != NULL )
        pLogTag = g_pAppName;
#else
    const CHAR* pLogTag = "SnapdragonToolkit";
#endif

    if( strPrefix )
    {
        __android_log_write(ANDROID_LOG_INFO, pLogTag, strPrefix);
        FrmAdrenoNativeActivityLog( strPrefix );
    }
    if( strMessage )
    {
        __android_log_write(ANDROID_LOG_INFO, pLogTag, strMessage);
        FrmAdrenoNativeActivityLog( strPrefix );
    }
    if( strPostFix )
    {
        __android_log_write(ANDROID_LOG_INFO, pLogTag, strPostFix);
        FrmAdrenoNativeActivityLog( strPrefix );
    }

}

//--------------------------------------------------------------------------------------
// Name: FrmLogMessage()
// Desc: Platform-dependent debug spew functions
//--------------------------------------------------------------------------------------
const CHAR* FrmGetMessageLog()
{
    return g_log;
}


//--------------------------------------------------------------------------------------
// Name: FrmGetTime()
// Desc: Platform-dependent function to get the current time (in seconds).
//--------------------------------------------------------------------------------------
FLOAT32 FrmGetTime()
{
	static BOOL		bInitialized = FALSE;
	static clock_t	baseTime;

	if ( FALSE == bInitialized )
	{
		baseTime = clock();
		bInitialized = TRUE;
		return 0.0f;
	}

	FLOAT32 fAppTime = (FLOAT32)(clock() - baseTime) / (FLOAT32)CLOCKS_PER_SEC;
	return fAppTime;
}

//--------------------------------------------------------------------------------------
// Name: FrmOpenConsole()
// Desc: Platform-dependent function to open a console
//--------------------------------------------------------------------------------------
int FrmOpenConsole()
{
	return 1;
}

#ifdef ANDROID_CL
//--------------------------------------------------------------------------------------
// Name: FrmAndroidUpdateImage()
// Desc: Update image in android UI
//--------------------------------------------------------------------------------------
VOID FrmAndroidUpdateImage( int viewID, const CHAR* path )
{
    if( g_pApp == NULL )
        return;

    // The AdrenoNativeActivity.java class has a method logMessage() that we call here
    // so that it can display the results of running the application on the screen.
    JavaVM* vm = g_pApp->activity->vm;
    JNIEnv* env;

    vm->AttachCurrentThread( &env, 0 );

    jobject me = g_pApp->activity->clazz;
    jclass acl = env->GetObjectClass( me );
    jmethodID updateImg = env->GetMethodID(acl, "updateImage", "(ILjava/lang/String;)V");

    //jstring jid = env->NewStringUTF( viewID );
    jstring jpath = env->NewStringUTF( path );

    env->CallVoidMethod(me, updateImg, viewID, jpath);

    //env->DeleteLocalRef(jid);
    env->DeleteLocalRef(jpath);

    vm->DetachCurrentThread();
}
#endif // ANDROID_CL

