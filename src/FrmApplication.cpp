// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmApplication.h"
#include "FrmInput.h"
#include "FrmUtils.h"
#include <cstring>

#ifdef _OGLES3
#include "OpenGLES/FrmGLES3.h"
#endif

//--------------------------------------------------------------------------------------
// Name: CFrmApplication()
// Desc: Construction the application base class. The default values can be overrided
//       by the derived class.
//--------------------------------------------------------------------------------------
#ifndef WINAPI_FAMILY
CFrmApplication::CFrmApplication( const CHAR* strName, FRM_ES_VER ESVersion )
#else
CFrmApplication::CFrmApplication( const CHAR* strName )
#endif
{ 
    m_strName                 = strName;


    // Set the initial display properties (matches Android MSM8660 landscape)
    m_nWidth                  = 765;
    m_nHeight                 = 480;

    // Set default values to be used during initialization.
#if defined(_WIN32) || defined(__APPLE__)
    m_nNumRedChannelBits      = 8;
    m_nNumGreenChannelBits    = 8;
    m_nNumBlueChannelBits     = 8;
    m_nNumAlphaChannelBits    = 8;
    m_nNumDepthBufferBits     = 24;
    m_nNumStencilBufferBits   = 8;
#else
	m_nNumRedChannelBits      = 5;
	m_nNumGreenChannelBits    = 6;
	m_nNumBlueChannelBits     = 7;
	m_nNumAlphaChannelBits    = 0;
	m_nNumDepthBufferBits     = 16;
	m_nNumStencilBufferBits   = 8;
#endif
    m_nNumMultiSampleBuffers  = 0;
    m_nNumSamplesPerPixel     = 0;
    m_bDoubleBuffer           = TRUE;


#ifndef WINAPI_FAMILY
    m_OpenGLESVersion		  = ESVersion;
	m_eglDisplay              = NULL;
    m_eglConfig               = NULL;
    m_eglContextGL            = NULL;
    m_eglContextVG            = NULL;
    m_eglSurface              = NULL;
#endif

    m_Input.m_nButtons        = 0;
    m_Input.m_nLastButtons    = 0;
    m_Input.m_nPressedButtons = 0;
    m_Input.m_nPointerState   = 0;

	// Reset sensor data
	m_Input.ResetAccelerometer();
	m_Input.ResetGyroscope();
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmApplication()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmApplication::~CFrmApplication()
{
    // Destroy the render context
#ifndef WINAPI_FAMILY
    DestroyRenderContext();
#endif
}


//--------------------------------------------------------------------------------------
// Name: DestroyRenderContext()
// Desc: Cleanup EGL.
//--------------------------------------------------------------------------------------
#ifndef WINAPI_FAMILY
VOID CFrmApplication::DestroyRenderContext()
{
    if( m_eglDisplay )
    {
        eglMakeCurrent( m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
        eglDestroyContext( m_eglDisplay, m_eglContextGL );
        eglDestroyContext( m_eglDisplay, m_eglContextVG );
        eglDestroySurface( m_eglDisplay, m_eglSurface );
        eglTerminate( m_eglDisplay );

        m_eglContextGL = 0;
        m_eglContextVG = 0;
        m_eglSurface   = 0;
        m_eglDisplay   = 0;
    }
}


//--------------------------------------------------------------------------------------
// Name: SwapDrawBuffers()
// Desc: Swap color buffers.
//--------------------------------------------------------------------------------------
VOID CFrmApplication::SwapDrawBuffers()
{
    if( m_bDoubleBuffer )
        eglSwapBuffers( m_eglDisplay, m_eglSurface );
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: Create a drawing context.
//--------------------------------------------------------------------------------------
BOOL CFrmApplication::CreateRenderContext( NativeWindowType hWindow,
                                           NativeDisplayType hDisplayType )
{
    // Get the display
    m_eglDisplay = eglGetDisplay( hDisplayType );

    if( m_eglDisplay == EGL_NO_DISPLAY )
    {
        FrmLogMessage( "ERROR: eglGetDisplay() failed.\n");
        return FALSE;
    }

    // Set our EGL API to OpenGL ES
    if ( eglBindAPI( EGL_OPENGL_ES_API ) == EGL_FALSE )
    {
        FrmLogMessage("eglBindAPI FAILED\n");
        return false;
    }

    // Initialize EGL
    EGLint nMajorVersion, nMinorVersion;
    if( FALSE == eglInitialize( m_eglDisplay, &nMajorVersion, &nMinorVersion ) )
    {
        FrmLogMessage( "ERROR: eglInitialize failed.\n" );
        return FALSE;
    }

    // Get configs
    EGLint nNumConfigs;
    if( FALSE == eglGetConfigs( m_eglDisplay, NULL, 0, &nNumConfigs ) )
    {
        FrmLogMessage( "ERROR: eglGetConfigs failed.\n" );
        return FALSE;
    }

    // Verify that the app is running on a display with sufficient bit depth
    if(strcmp(_CRT_STRINGIZE(PLATFORM), "OPENGLES") == 0)
    {
        EGLint nBPP = m_nNumRedChannelBits  + m_nNumGreenChannelBits +
                      m_nNumBlueChannelBits + m_nNumAlphaChannelBits;

        // Get the first config
        EGLConfig config;
        if( FALSE == eglGetConfigs( m_eglDisplay, &config, 1, &nNumConfigs ) )
            return FALSE;

        // Get the first config's Color buffer size
        EGLint nBuffSize;
        eglGetConfigAttrib( m_eglDisplay, config, EGL_BUFFER_SIZE, &nBuffSize );

        // Verify that the app is running on a 32-bit display
        if( nBuffSize < nBPP )
        {
            FrmLogMessage( "ERROR: Could not acquire an EGL configuration with sufficient bit depth for the requested size.\n" );
            return FALSE;
        }
    }

    // Build the attibute list
    EGLint ConfigAttribs[] = 
    { 
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES_BIT,
        EGL_RED_SIZE,		 (EGLint)m_nNumRedChannelBits,
        EGL_GREEN_SIZE,	     (EGLint)m_nNumGreenChannelBits,
        EGL_BLUE_SIZE,	     (EGLint)m_nNumBlueChannelBits,
        EGL_ALPHA_SIZE,	     (EGLint)m_nNumAlphaChannelBits,
        EGL_DEPTH_SIZE,	     (EGLint)m_nNumDepthBufferBits,
        EGL_STENCIL_SIZE,    (EGLint)m_nNumStencilBufferBits,
        EGL_SAMPLE_BUFFERS,  (EGLint)m_nNumMultiSampleBuffers,
        EGL_SAMPLES,		 (EGLint)m_nNumSamplesPerPixel,
        EGL_SURFACE_TYPE,   EGL_WINDOW_BIT,
        EGL_NONE
    };

    // Create space for the config list
    EGLConfig*  pEglConfigList = NULL;
    pEglConfigList = (EGLConfig*)malloc(nNumConfigs * sizeof(EGLConfig));
    if(pEglConfigList == NULL)
    {
        FrmLogMessage( "ERROR: Unable to allocate memory for EGL Config List.\n" );
        return FALSE;
    }


    // Choose config based on the requested attributes
    if( FALSE == eglChooseConfig( m_eglDisplay, ConfigAttribs, pEglConfigList, nNumConfigs, &nNumConfigs ) )
    {
        FrmLogMessage( "ERROR: eglChooseConfig failed.\n" );
        return FALSE;
    }

    INT32 nConfig = 0;
    BOOL bFoundConfig = FALSE;

    // Try each configuration to see if it matches our attribute list
    while (nConfig < nNumConfigs && !bFoundConfig)
    {
        EGLint nAttribValue = 0;
        UINT32 nAttrib=0; 

        // First check if this configuration supports our surface type
        if ( eglGetConfigAttrib(m_eglDisplay, pEglConfigList[nConfig], EGL_SURFACE_TYPE, &nAttribValue) == EGL_FALSE )
        {
            FrmLogMessage( "ERROR: eglGetConfigAttrib(1) failed.\n" );
        }
        bFoundConfig = (nAttribValue & EGL_WINDOW_BIT) != 0;

        // Then check the remaining requested attributes
        while (ConfigAttribs[nAttrib] != EGL_NONE && bFoundConfig)
        {
            if ( eglGetConfigAttrib(m_eglDisplay, pEglConfigList[nConfig], ConfigAttribs[nAttrib], &nAttribValue) == EGL_FALSE )
            {
                FrmLogMessage( "ERROR: eglGetConfigAttrib(2) failed.\n" );
            }
            if (ConfigAttribs[nAttrib + 1] != nAttribValue)
            {
                // [Jedi voice] "This is not the config you are looking for"
                bFoundConfig = FALSE;
            }
            nAttrib += 2;
        }

        // If we have not found a matching config, try the next one
        if (!bFoundConfig)
        {
            nConfig++;
        }
    }

    if (!bFoundConfig)
    {
        // None of the configs match our requested attribute list
        FrmLogMessage( "ERROR: Can't find a matching EGL configuration.\n" );

        // We could panic and return false, or just try to pick the first config 
        // that has a minimal match of attributes. 
        // TODO: determine the correct path (probably want to fail on device)
        nConfig = 0;
        if ( eglChooseConfig( m_eglDisplay, ConfigAttribs, pEglConfigList, 1, &nNumConfigs ) == EGL_FALSE )
        {
            free(pEglConfigList);
            FrmLogMessage( "ERROR: eglChooseConfig(2) failed.\n" );
            return false;
        }
    }

    // Create a window surface
    m_eglSurface = eglCreateWindowSurface( m_eglDisplay, pEglConfigList[nConfig], hWindow, NULL );
    if( EGL_NO_SURFACE == m_eglSurface )
    {
        free(pEglConfigList);
        FrmLogMessage( "ERROR: eglCreateWindowSurface failed.\n" );
        return FALSE;
    }

    // Note: eventually we'll want to be able to have both GLES and VG render contexts similtaneously

    // Create the OpenGL rendering context
    if(strcmp(_CRT_STRINGIZE(PLATFORM), "OPENGLES") == 0)
    {
        EGLint ContextAttribs[] =
        { 
            EGL_CONTEXT_CLIENT_VERSION, 2, 
            EGL_NONE 
        };
        m_eglContextGL = eglCreateContext( m_eglDisplay, pEglConfigList[nConfig], EGL_NO_CONTEXT, ContextAttribs );
    }

    // No longer need the EGL config list
    free(pEglConfigList);

    // Set the swap interval so that FPS is not tied to the monitor refresh rate
    eglSwapInterval( m_eglDisplay, 0 );

    // grab ES3 pointers if ES3 context defined
#ifdef _OGLES3
    RetrieveES2ExtES3Funcs();
#endif
    return TRUE;
}
#endif
