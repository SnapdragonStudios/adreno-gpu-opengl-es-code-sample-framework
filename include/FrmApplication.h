// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_APPLICATION_H_
#define _FRM_APPLICATION_H_

#include "FrmPlatform.h"
#ifndef WINAPI_FAMILY
    #include <EGL/egl.h>
#endif
#include "FrmInput.h"


//--------------------------------------------------------------------------------------
// Name: class CFrmApplication()
// Desc: Application class
//--------------------------------------------------------------------------------------
class CFrmApplication
{
public:
    virtual BOOL Initialize() = 0;
    virtual BOOL Resize() { return TRUE; }
    virtual VOID Destroy() = 0;
    virtual VOID Update() = 0;
    virtual VOID Render() = 0;

#ifndef WINAPI_FAMILY
    BOOL CreateRenderContext( NativeWindowType hWindow, NativeDisplayType hDisplayType );
    VOID DestroyRenderContext();
    VOID SwapDrawBuffers();
#endif

#ifndef WINAPI_FAMILY
	// Decide at runtime which version of OpenGLES to use
	enum FRM_ES_VER
	{
		GLES2,
		GLES3
	};
    CFrmApplication( const CHAR* strName, FRM_ES_VER ESVersion = GLES2 );
#else
	CFrmApplication( const CHAR* strName );
#endif

    virtual ~CFrmApplication();

public:
    const CHAR* m_strName;
    INT32       m_nWidth;
    INT32       m_nHeight;

    // EGL properties
    UINT32      m_nNumRedChannelBits;
    UINT32      m_nNumGreenChannelBits;
    UINT32      m_nNumBlueChannelBits;
    UINT32      m_nNumAlphaChannelBits;
    UINT32      m_nNumDepthBufferBits;
    UINT32      m_nNumStencilBufferBits;
    UINT32      m_nNumMultiSampleBuffers;
    UINT32      m_nNumSamplesPerPixel;
    BOOL        m_bDoubleBuffer;

#ifndef WINAPI_FAMILY
	FRM_ES_VER	m_OpenGLESVersion;
    EGLDisplay  m_eglDisplay;
    EGLConfig   m_eglConfig;
    EGLContext  m_eglContextGL;
    EGLContext  m_eglContextVG;
    EGLSurface  m_eglSurface;
#else
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_windowRenderTargetView;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_windowDepthStencilView;
#endif

    FRM_INPUT   m_Input;
};


//--------------------------------------------------------------------------------------
// Name: FrmCreateApplicationInstance()
// Desc: Global function to create the application instance
//--------------------------------------------------------------------------------------
extern CFrmApplication* FrmCreateApplicationInstance();


#endif // _FRM_APPLICATION_H_
