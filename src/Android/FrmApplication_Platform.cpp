//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmPlatform.h"
#include "FrmApplication.h"
#include "FrmUtils.h"
#include <cstring>

// Calls used for communication with remote PC
BOOL InitializeTCPServer();
BOOL ResetTCPServer();
BOOL DestroyTCPServer();
BOOL GetTCPServerMessage(char *msgBuf, int sizeMsgBuf);
static void* SocketThreadFunc( void *pArg );

class CFrmAppContainer
{
public:
	CFrmAppContainer();
	~CFrmAppContainer();

	BOOL Initialize();
	BOOL Run();
	VOID Destroy();
	
	VOID HandleCommand(int32_t command);
	int32_t HandleInput(AInputEvent* event);
	
	VOID SetAndroidApp(android_app* pApp)	{ m_pAndroidApp = pApp; }
	
protected:
	BOOL InitializeEgl();
	VOID DestroyEgl();

	CFrmApplication*	m_pApplication;
	android_app*		m_pAndroidApp;
	EGLDisplay			m_eglDisplay;
	EGLSurface			m_eglSurface;
	EGLContext			m_eglContext;

	// Sensor members (for accel and gyro)
    ASensorManager	   *m_sensorManager = nullptr;    
	const ASensor	   *m_accelSensor = nullptr;    
	ASensorEventQueue  *m_accelEventQueue = nullptr;
	const ASensor	   *m_gyroscopeSensor = nullptr;    
	ASensorEventQueue  *m_gyroscopeEventQueue = nullptr;
};


//--------------------------------------------------------------------------------------
// Name: CFrmAppContainer()
// Desc: Constructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::CFrmAppContainer()
: m_pApplication(NULL)
, m_pAndroidApp(NULL)
, m_eglDisplay(EGL_NO_DISPLAY)
, m_eglSurface(EGL_NO_SURFACE)
, m_eglContext(EGL_NO_CONTEXT)
{}


//--------------------------------------------------------------------------------------
// Name: ~CFrmAppContainer()
// Desc: Destructor
//--------------------------------------------------------------------------------------
CFrmAppContainer::~CFrmAppContainer()
{
	if(m_pApplication)
	{
		m_pApplication->Destroy();
		delete m_pApplication;
	}
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Set up application window and EGL context
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Initialize()
{
	m_pApplication = FrmCreateApplicationInstance();
	if ( NULL == m_pApplication )
	{
		return FALSE;
	}

	InitializeEgl();

#ifdef _OGLES3
    RetrieveES2ExtES3Funcs();
#endif

    m_pApplication->m_eglDisplay = m_eglDisplay;
    m_pApplication->m_eglSurface = m_eglSurface;
    m_pApplication->m_eglContextGL = m_eglContext;

	// TODO: Move to app container class when everything works
	InitializeTCPServer();
	m_pApplication->m_nWidth = ANativeWindow_getWidth(m_pAndroidApp->window);     // NATIVE_APP_WIDTH;
	m_pApplication->m_nHeight = ANativeWindow_getHeight(m_pAndroidApp->window);   // NATIVE_APP_HEIGHT; 

	if( ( FALSE == m_pApplication->Initialize() ) ||
		( FALSE == m_pApplication->Resize() ) )
	{
		FrmLogMessage( "ERROR: The application failed to initialize, the application will now exit." );

		return FALSE;
	}

	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
void CFrmAppContainer::Destroy()
{
	DestroyEgl();
	DestroyTCPServer();

	// destroy the process
	exit(0);
}


//--------------------------------------------------------------------------------------
// Name: Run()
// Desc: Set up application window and EGL context
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::Run()
{
#define ADRENO_SENSOR_MANAGER 0 //SensorManager is deprecated
#if ADRENO_SENSOR_MANAGER
	// Prepare to monitor accelerometer and gyroscope
	m_sensorManager       = ASensorManager_getInstance();    
	m_accelSensor         = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_ACCELEROMETER);
	m_accelEventQueue     = ASensorManager_createEventQueue(m_sensorManager, m_pAndroidApp->looper, LOOPER_ID_USER, NULL, NULL);
	m_gyroscopeSensor     = ASensorManager_getDefaultSensor(m_sensorManager, ASENSOR_TYPE_GYROSCOPE);
	m_gyroscopeEventQueue = ASensorManager_createEventQueue(m_sensorManager, m_pAndroidApp->looper, (LOOPER_ID_USER + 1), NULL, NULL);
#endif//ADRENO_SENSOR_MANAGER

	while (1)
	{
		int ident;
		int events;
		android_poll_source* pSource;

		while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&pSource)) >= 0)
		{
			if (pSource != NULL)
			{
				pSource->process(m_pAndroidApp, pSource);
			}

			// If a sensor has data, process it now.

			// ACCELOMETER
			if (ident == LOOPER_ID_USER) 
			{                
				if (m_accelSensor != NULL) 
				{                    
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(m_accelEventQueue, &event, 1) > 0) 
					{
						m_pApplication->m_Input.m_AccelSupported = TRUE;

						FRMVECTOR3 &ARaw = m_pApplication->m_Input.m_AccelRaw;
						
						// Cache old ARaw values
						FRMVECTOR3 ARawOld = ARaw;
						
						ARaw.v[0] = event.vector.x; 
						ARaw.v[1] = event.vector.y; 
						ARaw.v[2] = event.vector.z; 

						FRMVECTOR3 &AGravity = m_pApplication->m_Input.m_AccelGravity;
						FLOAT32 HPFK  = m_pApplication->m_Input.m_AccelHPFK;
						FLOAT32 HPFKI = 1.0f - HPFK;

						// High-pass Filter the Gravity value and remove from raw input
 						AGravity.v[0] = (HPFK * AGravity.v[0]) + (HPFKI * ARaw.v[0]);	
 						ARaw.v[0] -= AGravity.v[0];
 						AGravity.v[1] = (HPFK * AGravity.v[1]) + (HPFKI * ARaw.v[1]);	
 						ARaw.v[1] -= AGravity.v[1];
 						AGravity.v[2] = (HPFK * AGravity.v[2]) + (HPFKI * ARaw.v[2]);	
 						ARaw.v[2] -= AGravity.v[2];

						// NOTE: The following calculations are not accurate to what they
						// are being called, but are useful values to use with an application

						// Smoothed out acceleration value (useful for detecting shaking and 
						// for calbrating gyro etc.)
						FRMVECTOR3 &ASmooth = m_pApplication->m_Input.m_AccelSmooth;
//						ASmooth.v[0] += 0.00833333f * (ARaw.v[0] - ARawOld.v[0]);
//						ASmooth.v[1] += 0.00833333f * (ARaw.v[1] - ARawOld.v[1]);
//						ASmooth.v[2] += 0.00833333f * (ARaw.v[2] - ARawOld.v[2]);
						ASmooth.v[0] += ARaw.v[0] - ARawOld.v[0];
						ASmooth.v[1] += ARaw.v[1] - ARawOld.v[1];
						ASmooth.v[2] += ARaw.v[2] - ARawOld.v[2];

						// Velocity value (useful for applying force to objects in application
						// that will smoothly move across the screen etc.)
						FRMVECTOR3 &AVel = m_pApplication->m_Input.m_AccelVelocity;
						AVel.v[0] += 0.00833333f * ASmooth.v[0];
						AVel.v[1] += 0.00833333f * ASmooth.v[1];
						AVel.v[2] += 0.00833333f * ASmooth.v[2];
					}					
				}            
			}

			// GYROSCOPE
			if (ident == LOOPER_ID_USER + 1) 
			{                
				// Gyro
				if (m_gyroscopeSensor != NULL) 
				{                    
					ASensorEvent event;
					while (ASensorEventQueue_getEvents(m_gyroscopeEventQueue, &event, 1) > 0) 
					{
						m_pApplication->m_Input.m_GyroSupported = TRUE;

						float x = event.vector.x; 
						float y = event.vector.y;
						float z = event.vector.z;

						FRMVECTOR3 &GRaw = m_pApplication->m_Input.m_GyroRaw;
						FRMVECTOR3 GRawOld = GRaw;
						GRaw.v[0] = x;
						GRaw.v[1] = y;
						GRaw.v[2] = z;

						FRMVECTOR3 &GOffset = m_pApplication->m_Input.m_GyroOffset;
						float xo = GOffset.v[0] * 2.0f;
						float yo = GOffset.v[1] * 2.0f;
						float zo = GOffset.v[2] * 2.0f;
					
						// a(k) = a(k-1) + T*0.5*(da(k) + da(k-1))
						FRMVECTOR3 &GSmooth = m_pApplication->m_Input.m_GyroSmooth;
						GSmooth.v[0] +=  0.00833333f * (GRawOld.v[0] + GRaw.v[0] - xo);
						GSmooth.v[1] +=  0.00833333f * (GRawOld.v[1] + GRaw.v[1] - yo);
						GSmooth.v[2] +=  0.00833333f * (GRawOld.v[2] + GRaw.v[2] - zo);
					}                
				}            
			}

			if (m_pAndroidApp->destroyRequested != 0)
			{
				Destroy();
				return TRUE;
			}
		}
		
		//------------------------------------------------------------
		// Get mssage and forward it to all listeners (for now, we are 
		// only interested in mapping into kb and mouse input)
		char msgbuf[256];
		if(GetTCPServerMessage(msgbuf, 256) == TRUE)
		{
			// Check header to make sure this is intended for uss
			if ((msgbuf[0] != 'S')||(msgbuf[1] != '2')||(msgbuf[2] != 'D')||(msgbuf[3] != 'K') )
				continue;

			if(msgbuf[4] == 0) // MSG TYPE: KEY (0)
			{
				UINT32 nButtonMask = 0;

				switch(msgbuf[5])
				{
					case '1': nButtonMask = FRM_INPUT::KEY_1; break;
					case '2': nButtonMask = FRM_INPUT::KEY_2; break;
					case '3': nButtonMask = FRM_INPUT::KEY_3; break;
					case '4': nButtonMask = FRM_INPUT::KEY_4; break;
					case '5': nButtonMask = FRM_INPUT::KEY_5; break;
					case '6': nButtonMask = FRM_INPUT::KEY_6; break;
					case '7': nButtonMask = FRM_INPUT::KEY_7; break;
					case '8': nButtonMask = FRM_INPUT::KEY_8; break;
					case '9': nButtonMask = FRM_INPUT::KEY_9; break;
					case '0': nButtonMask = FRM_INPUT::KEY_0; break;
				}

				if( nButtonMask)
				{
					UINT32 nButtonDown = msgbuf[6];

					if(nButtonDown)
						m_pApplication->m_Input.m_nButtons |= nButtonMask;
					else
						m_pApplication->m_Input.m_nButtons &= ~nButtonMask;
				}
			}
			else
			if(msgbuf[4] == 1) // MSG TYPE: MOUSE (1)
			{
				char buf[256];
				bool bDown = msgbuf[5];
				bool bMove = msgbuf[6];
				float X = ((float)msgbuf[7] - 100.0f) * 0.01f;
				float Y = ((float)msgbuf[8] - 100.0f) * 0.01f; Y = -Y;

				FRMVECTOR2 vPointerPosition;
				vPointerPosition.x = X;
				vPointerPosition.y = Y;

				m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;

				if(bDown && !bMove)
				{
					m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
					m_pApplication->m_Input.m_nPointerState |= FRM_INPUT::POINTER_PRESSED;

				}
				else if(bDown && bMove)
				{
					m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_DOWN;
				}
				else
				{
					m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_RELEASED;
				}
			}
		}
		//------------------------------------------------------------

		// Present the scene
		if(m_eglDisplay == NULL)
			continue;

		// Update and render the application
		m_pApplication->Update();
		m_pApplication->Render();
		eglSwapBuffers(m_eglDisplay, m_eglSurface);
	}
}


//--------------------------------------------------------------------------------------
// Name: HandleCommand()
// Desc: Filter and respond to Android commands
//--------------------------------------------------------------------------------------
VOID CFrmAppContainer::HandleCommand(int32_t cmd)
{
	switch (cmd)
	{
	case APP_CMD_INIT_WINDOW:
		if (m_pAndroidApp->window != NULL)
		{
			if ( FALSE == Initialize() )
			{
				// exit
				exit(1);
			}
		}
		break;

	case APP_CMD_TERM_WINDOW:
		Destroy();
		break;

    case APP_CMD_GAINED_FOCUS:            
		// When our app gains focus, we start monitoring the sensors.            
		if (m_accelSensor != NULL)
		{                
			ASensorEventQueue_enableSensor(m_accelEventQueue, m_accelSensor);

			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(m_accelEventQueue, m_accelSensor, (1000L/60)*1000);
		}            

		if (m_gyroscopeSensor != NULL) 
		{                
			ASensorEventQueue_enableSensor(m_gyroscopeEventQueue, m_gyroscopeSensor);

			// We'd like to get 60 events per second (in us).
			ASensorEventQueue_setEventRate(m_gyroscopeEventQueue, m_gyroscopeSensor, (1000L/60)*1000);
		}
		break;        
	
	case APP_CMD_LOST_FOCUS:            
		// When our app loses focus, we stop monitoring the sensors.            
		// This is to avoid consuming battery while not being used.
		if (m_accelSensor != NULL)
		{                
			ASensorEventQueue_disableSensor(m_accelEventQueue, m_accelSensor); 
		}

		if (m_gyroscopeSensor != NULL) 
		{                
			ASensorEventQueue_disableSensor(m_gyroscopeEventQueue, m_gyroscopeSensor); 
		}
		break;

	default: 
		break;
	}
}

//--------------------------------------------------------------------------------------
// Name: HandleInput()
// Desc: Filter and respond to Android input events
//       Currently we are only capturing touch-screen events
//		 We normalize for each axis from -1.0f -> 1.0f
//--------------------------------------------------------------------------------------
int32_t CFrmAppContainer::HandleInput(AInputEvent* event)
{
	if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) 
	{
		int32_t nAction = AMotionEvent_getAction(event);
				
		float x =   ((AMotionEvent_getX(event,0) / (float)m_pApplication->m_nWidth ) * 2.0f) - 1.0f;
		if (x < -1.0f) x = -1.0f;
		else if (x > 1.0f) x = 1.0f;

		float y = -(((AMotionEvent_getY(event,0) / (float)m_pApplication->m_nHeight) * 2.0f) - 1.0f);
		if (y < -1.0f) y = -1.0f;
		else if (y > 1.0f) y = 1.0f;

		float pressure = AMotionEvent_getPressure(event, 0);

		FRMVECTOR2 vPointerPosition;
		vPointerPosition.x = x;
		vPointerPosition.y = y;

		// Set location
		m_pApplication->m_Input.m_vPointerPosition = vPointerPosition;

		// Set press logic
		if(nAction == AMOTION_EVENT_ACTION_DOWN)
		{
			m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
			m_pApplication->m_Input.m_nPointerState |= FRM_INPUT::POINTER_PRESSED;
		}
		if(nAction == AMOTION_EVENT_ACTION_UP)
		{
			m_pApplication->m_Input.m_nPointerState = FRM_INPUT::POINTER_RELEASED;
		}
		else
		{
			m_pApplication->m_Input.m_nPointerState  = FRM_INPUT::POINTER_DOWN;
		}

        return 1;
	}

    return 0;
}

//--------------------------------------------------------------------------------------
// Name: InitializeEgl()
// Desc: Set up egl context
//--------------------------------------------------------------------------------------
BOOL CFrmAppContainer::InitializeEgl()
{
	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	eglInitialize(display, 0, 0);

	EGLConfig config;
	EGLint numConfigs;
	EGLint format;
	EGLint configAttribs[] = 
	{
		EGL_SURFACE_TYPE,		EGL_WINDOW_BIT,
		EGL_RED_SIZE,			(EGLint)m_pApplication->m_nNumRedChannelBits,
		EGL_GREEN_SIZE,	    	(EGLint)m_pApplication->m_nNumGreenChannelBits,
		EGL_BLUE_SIZE,	    	(EGLint)m_pApplication->m_nNumBlueChannelBits,
		EGL_ALPHA_SIZE,			(EGLint)m_pApplication->m_nNumAlphaChannelBits,
		EGL_DEPTH_SIZE,			(EGLint)m_pApplication->m_nNumDepthBufferBits,
		EGL_STENCIL_SIZE,       (EGLint)m_pApplication->m_nNumStencilBufferBits,
#ifdef _OGLES3
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES3_BIT_KHR,     // this bit opens access to ES3 functions on QCOM hardware pre-Android support for ES3
#else
        EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES2_BIT,
#endif
		EGL_NONE
	};
	
	eglChooseConfig(display, configAttribs, &config, 1, &numConfigs);
	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	ANativeWindow_setBuffersGeometry(m_pAndroidApp->window, ANativeWindow_getWidth(m_pAndroidApp->window), ANativeWindow_getHeight(m_pAndroidApp->window), format);
	
	EGLSurface surface = eglCreateWindowSurface(display, config, m_pAndroidApp->window, NULL);
	
	EGLint contextAttribs[] = 
    { 
        EGL_CONTEXT_CLIENT_VERSION,  2, 
        EGL_NONE
    };


	EGLContext context = eglCreateContext(display, config, NULL, contextAttribs);
  
	
	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		return FALSE;
	}
	
	m_eglDisplay = display;
	m_eglContext = context;
	m_eglSurface = surface;
	
	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: DestroyEgl()
// Desc: Destroy egl context
//--------------------------------------------------------------------------------------
void CFrmAppContainer::DestroyEgl()
{
	if (m_eglDisplay != EGL_NO_DISPLAY)
	{
		eglMakeCurrent(m_eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		if (m_eglContext != EGL_NO_CONTEXT)
		{
			eglDestroyContext(m_eglDisplay, m_eglContext);
		}
		if(m_eglSurface != EGL_NO_SURFACE)
		{
			eglDestroySurface(m_eglDisplay, m_eglSurface);
		}
		eglTerminate(m_eglDisplay);
	}

	m_eglDisplay = EGL_NO_DISPLAY;
	m_eglContext = EGL_NO_CONTEXT;
	m_eglSurface = EGL_NO_SURFACE;
}
	
//--------------------------------------------------------------------------------------
// Name: InputCallback()
// Desc: Android input callback
//--------------------------------------------------------------------------------------
static int32_t InputCallback(struct android_app* pApp, AInputEvent* pEvent)
{
	CFrmAppContainer* pAppContainer = (CFrmAppContainer*)pApp->userData;
	return pAppContainer->HandleInput(pEvent);
}

//--------------------------------------------------------------------------------------
// Name: CommandCallback()
// Desc: Android command handler
//--------------------------------------------------------------------------------------
static void CommandCallback(android_app* pApp, int32_t cmd)
{
	CFrmAppContainer* pAppContainer = (CFrmAppContainer*)pApp->userData;
	pAppContainer->HandleCommand(cmd);
}

extern AAssetManager* g_pAssetManager;

void handle_cmd(android_app *pApp, int32_t cmd) {
}

//--------------------------------------------------------------------------------------
// Name: android_main()
// Desc: Application entry point
//--------------------------------------------------------------------------------------
void android_main(android_app* pApp)
{
	CFrmAppContainer appContainer;
	appContainer.SetAndroidApp(pApp);
	pApp->userData = &appContainer;
	pApp->onAppCmd = CommandCallback;
	pApp->onInputEvent = InputCallback;

	g_pAssetManager = pApp->activity->assetManager;
	
	appContainer.Run();
}

//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------
// CLIENT/SERVER CODE
//--------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------

const static char *sHostName = "localhost";
const static int   sPortNum = 54321;

// Handle to listener thread for starting up communication
static pthread_t sSocketConnectThread = 0;
static bool      sConnectionOpen = FALSE;

// Handle to sockets
static int sSockfd    = -1; 
static int sNewsockfd = -1;

// Global message recieve
static char sMsg[256];
static int  sMsgSize = 0;

// Semaphore
static sem_t sMutex;


//--------------------------------------------------------------------------------------
// Name: InitializeTCPServer()
// Desc: Set up a communication bridge to the PC for sending TCP/IP packets
//--------------------------------------------------------------------------------------
BOOL InitializeTCPServer()
{
	// create, initialize semaphore 
 	if( sem_init(&sMutex,1,1) < 0)
 	{
		FrmLogMessage(NULL, "InitializeTCPServer: ERROR creating mutex\n", NULL);
		return FALSE;
 	}
	
	// If channel exists, destroy it
	DestroyTCPServer();

	// Start the recieve sockeet up
	ResetTCPServer();

	// invalidate connection
	sConnectionOpen = FALSE;

	// invalidate message
	sMsgSize = 0;

	// Launch socket thread
	if( sSocketConnectThread == 0 )
	{
		// Create the listener thread
		if( pthread_create( &sSocketConnectThread, NULL, SocketThreadFunc, NULL ) != 0 )
		{
			FrmLogMessage(NULL, "InitializeTCPServer: ERROR on SocketThread creation\n", NULL);
			
			// Make sure to shut down the main socket
			close(sSockfd);
			sSockfd = -1;

			return FALSE;
		}
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: ResetTCPServer()
// Desc: Reset the recieve socket
//--------------------------------------------------------------------------------------
BOOL ResetTCPServer()
{
	struct sockaddr_in serv_addr;

	// Open a socket to read from (SOCK_STREAM = TCP, SOCK_DGRAM = UDP)
	sSockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sSockfd < 0) 
	{
		FrmLogMessage(NULL, "ResetTCPServer: ERROR opening socket\n", NULL);
		return FALSE;
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(sPortNum);

	// Set this up before binding to properly tear down communications when exiting
	// applications
	BOOL bOptVal = TRUE;
	setsockopt(sSockfd, SOL_SOCKET, SO_REUSEADDR, (char *) &bOptVal, sizeof bOptVal);

	if (bind(sSockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		FrmLogMessage(NULL, "ResetTCPServer: ERROR on binding\n", NULL);
		return FALSE;
	}

	listen(sSockfd, 5);

	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: GetTCPServerMessage()
// Desc: Run the client messaging code
//--------------------------------------------------------------------------------------
BOOL GetTCPServerMessage(char *msgBuf, int sizeMsgBuf)
{
	// This may have closed down due to a lost socket (client may have gone down already)
	if ((sConnectionOpen) && (msgBuf) && (sizeMsgBuf > 0))
	{
		sem_wait(&sMutex);
		int msgSize = sMsgSize; 
		sem_post(&sMutex);

		if (msgSize > sizeMsgBuf) 
		{
			msgSize = sizeMsgBuf;
		}

		if(msgSize > 0)
		{
			memcpy((void *)msgBuf, (const void*)sMsg, (size_t)msgSize);				
			sem_wait(&sMutex);
			sMsgSize = 0;
			sem_post(&sMutex);

			return TRUE;
		}
	}

	return FALSE;
}

//--------------------------------------------------------------------------------------
// Name: DestroyTCPServer()
// Desc: Tear down the communication of the client
//--------------------------------------------------------------------------------------
BOOL DestroyTCPServer()
{
	close(sNewsockfd);
	sNewsockfd = -1;
	close(sSockfd);
	sSockfd = -1;

	if(sSocketConnectThread != 0)
	{
		pthread_kill(sSocketConnectThread, 0);
		sSocketConnectThread = 0;
	}

	return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: SocketThreadFunc()
// Desc: Run a thread to open up the connection outside of main thread
//--------------------------------------------------------------------------------------
static void* SocketThreadFunc( void *pArg )
{
	struct sockaddr_in cli_addr;
	socklen_t clilen;

	char msgBuffer[256];

	clilen = sizeof(cli_addr);

	while (1)
	{
		if( sConnectionOpen == FALSE )
		{
			FrmLogMessage(NULL, "SocketThreadFunc: WAITING FOR CONNECTION.\n", NULL);
			sNewsockfd = accept(sSockfd, (struct sockaddr *) &cli_addr, &clilen);

			if (sNewsockfd < 0) 
			{
				FrmLogMessage(NULL, "SocketThreadFunc: ERROR on accept", NULL);
				return NULL;
			}

			FrmLogMessage(NULL, "SocketThreadFunc: CONNECTION SUCCESSFUL.\n", NULL);

			// we were successful with connection, open it up
			sConnectionOpen = TRUE;
		}
		else
		{
			sem_wait(&sMutex);
			int msgSize = sMsgSize;
			sem_post(&sMutex);

			// Skip the read until sMsgSize has been set to 0 by the main thread
			if(msgSize == 0)
			{
				msgSize = read(sNewsockfd, msgBuffer ,255);

				if (msgSize == 0)
				{
					// something went wrong, reset!
					sConnectionOpen = FALSE;

					DestroyTCPServer();
					ResetTCPServer();
				}
				else
				{
					// copy message to main buffer (possibly to a msg queue)			
					memcpy((void *)sMsg, (const void*)msgBuffer, (size_t)msgSize);				

					sem_wait(&sMutex);
					sMsgSize = msgSize;
					sem_post(&sMutex);
				}
			}
		}
	}
	
	return NULL;
}
