// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmInput.h"


//--------------------------------------------------------------------------------------
// Name: GetButtonState()
// Desc: Process button input that changed since the last time this function was called.
//--------------------------------------------------------------------------------------
VOID FRM_INPUT::GetButtonState( UINT32* pnButtons, UINT32* pnPressedButtons )
{
    m_nPressedButtons = m_nButtons & (~m_nLastButtons);
    m_nLastButtons    = m_nButtons;

    if( pnPressedButtons )  (*pnPressedButtons)  = m_nPressedButtons;
    if( pnButtons )         (*pnButtons)         = m_nButtons;
}


//--------------------------------------------------------------------------------------
// Name: GetPointerState()
// Desc: Process pointer input that changed since the last time this function was called.
//--------------------------------------------------------------------------------------
VOID FRM_INPUT::GetPointerState( UINT32* pnPointerState, FRMVECTOR2* pvPointerPosition )
{
    if( pnPointerState )    (*pnPointerState)    = m_nPointerState;
    if( pvPointerPosition ) (*pvPointerPosition) = m_vPointerPosition;

    // Reset any one-time pointer state
    m_nPointerState &= ~FRM_INPUT::POINTER_PRESSED;
    m_nPointerState &= ~FRM_INPUT::POINTER_RELEASED;
}


//--------------------------------------------------------------------------------------
// Name: FrmGetInput()
// Desc: Process input that changed since the last time this function was called.
//--------------------------------------------------------------------------------------
VOID FrmGetInput( FRM_INPUT* pInput, UINT32* pnButtons, UINT32* pnPressedButtons )
{
    pInput->GetButtonState( pnButtons, pnPressedButtons );
}

//--------------------------------------------------------------------------------------
// Name: FrmGetInput()
// Desc: Process mouse input that changed since the last time this function was called.
//--------------------------------------------------------------------------------------
VOID FrmGetMouseInput(FRM_INPUT *pInput, UINT32* pnPointerState, FRMVECTOR2 *pvPointerPosition)
{
    pInput->GetPointerState(pnPointerState, pvPointerPosition);
}

//--------------------------------------------------------------------------------------
// Name: ResetAccelerometer()
// Desc: Resets the accelerometer variables
//--------------------------------------------------------------------------------------
VOID FRM_INPUT::ResetAccelerometer()
{
	m_AccelSupported     = FALSE;
	m_AccelRaw.v[0]      = 0.0f;
	m_AccelRaw.v[1]      = 0.0f;
	m_AccelRaw.v[2]      = 0.0f;
	m_AccelGravity.v[0]  = 0.0f;
	m_AccelGravity.v[1]  = 0.0f;
	m_AccelGravity.v[2]  = 0.0f;
	m_AccelSmooth.v[0]   = 0.0f;
	m_AccelSmooth.v[1]   = 0.0f;
	m_AccelSmooth.v[2]   = 0.0f;
	m_AccelVelocity.v[0] = 0.0f;
	m_AccelVelocity.v[1] = 0.0f;
	m_AccelVelocity.v[2] = 0.0f;
	m_AccelHPFK			 = 0.9f;	
}


//--------------------------------------------------------------------------------------
// Name: ResetGyroscope()
// Desc: Resets the gyroscope variables
//--------------------------------------------------------------------------------------
VOID FRM_INPUT::ResetGyroscope()
{
	m_GyroSupported    = FALSE;
	m_GyroRaw.v[0]     = 0.0f;
	m_GyroRaw.v[1]     = 0.0f;
	m_GyroRaw.v[2]     = 0.0f;
	m_GyroOffset.v[0]  = 0.0f;
	m_GyroOffset.v[1]  = 0.0f;
	m_GyroOffset.v[2]  = 0.0f;
	m_GyroSmooth.v[0]  = 0.0f;
	m_GyroSmooth.v[1]  = 0.0f;
	m_GyroSmooth.v[2]  = 0.0f;
}