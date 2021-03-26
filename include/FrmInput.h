// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_INPUT_H_
#define _FRM_INPUT_H_

#include "FrmPlatform.h"
#include "FrmMath.h"


//--------------------------------------------------------------------------------------
// Bit-field assignment for user input
//--------------------------------------------------------------------------------------
const UINT32 INPUT_KEY_1        = 0x00000001;
const UINT32 INPUT_KEY_2        = 0x00000002;
const UINT32 INPUT_KEY_3        = 0x00000004;
const UINT32 INPUT_KEY_4        = 0x00000008;
const UINT32 INPUT_KEY_5        = 0x00000010;
const UINT32 INPUT_KEY_6        = 0x00000020;
const UINT32 INPUT_KEY_7        = 0x00000040;
const UINT32 INPUT_KEY_8        = 0x00000080;
const UINT32 INPUT_KEY_9        = 0x00000100;
const UINT32 INPUT_KEY_0        = 0x00000200;
const UINT32 INPUT_KEY_STAR     = 0x00000400;
const UINT32 INPUT_KEY_HASH     = 0x00000800;
const UINT32 INPUT_LEFT_BUTTON  = 0x00001000;
const UINT32 INPUT_RIGHT_BUTTON = 0x00002000;
const UINT32 INPUT_KEY_CLEAR    = 0x00004000;
const UINT32 INPUT_KEY_EDIT     = 0x00008000;
const UINT32 INPUT_SELECT       = 0x00010000;
const UINT32 INPUT_DPAD_UP      = 0x00020000;
const UINT32 INPUT_DPAD_DOWN    = 0x00040000;
const UINT32 INPUT_DPAD_LEFT    = 0x00080000;
const UINT32 INPUT_DPAD_RIGHT   = 0x00100000;
const UINT32 INPUT_KEY_SEND     = 0x00200000;
const UINT32 INPUT_KEY_VOL_UP   = 0x00400000;
const UINT32 INPUT_KEY_VOL_DOWN = 0x00800000;
const UINT32 INPUT_KEY_EXTRA_A  = 0x01000000;
const UINT32 INPUT_KEY_EXTRA_B  = 0x02000000;
const UINT32 INPUT_KEY_EXTRA_C  = 0x04000000;
const UINT32 INPUT_KEY_EXTRA_D  = 0x08000000;
const UINT32 INPUT_KEY_EXTRA_E  = 0x10000000;
const UINT32 INPUT_KEY_EXTRA_F  = 0x20000000;
const UINT32 INPUT_KEY_EXTRA_G  = 0x40000000;
const UINT32 INPUT_KEY_EXTRA_H  = 0x80000000;


//--------------------------------------------------------------------------------------
// Name: struct FRM_INPUT
// Desc: Structure for polling user input
//--------------------------------------------------------------------------------------
struct FRM_INPUT
{
    // Button codes
    static const UINT32 NONE         = 0;
    static const UINT32 KEY_1        = (1UL <<  0);
    static const UINT32 KEY_2        = (1UL <<  1);
    static const UINT32 KEY_3        = (1UL <<  2);
    static const UINT32 KEY_4        = (1UL <<  3);
    static const UINT32 KEY_5        = (1UL <<  4);
    static const UINT32 KEY_6        = (1UL <<  5);
    static const UINT32 KEY_7        = (1UL <<  6);
    static const UINT32 KEY_8        = (1UL <<  7);
    static const UINT32 KEY_9        = (1UL <<  8);
    static const UINT32 KEY_0        = (1UL <<  9);
    static const UINT32 KEY_STAR     = (1UL << 10);
    static const UINT32 KEY_HASH     = (1UL << 11);
    static const UINT32 LEFT_BUTTON  = (1UL << 12);
    static const UINT32 RIGHT_BUTTON = (1UL << 13);
    static const UINT32 KEY_CLEAR    = (1UL << 14);
    static const UINT32 KEY_EDIT     = (1UL << 15);
    static const UINT32 SELECT       = (1UL << 16);
    static const UINT32 DPAD_UP      = (1UL << 17);
    static const UINT32 DPAD_DOWN    = (1UL << 18);
    static const UINT32 DPAD_LEFT    = (1UL << 19);
    static const UINT32 DPAD_RIGHT   = (1UL << 20);
    static const UINT32 KEY_SEND     = (1UL << 21);
    static const UINT32 KEY_VOL_UP   = (1UL << 22);
    static const UINT32 KEY_VOL_DOWN = (1UL << 23);
    static const UINT32 KEY_EXTRA_A  = (1UL << 24);
    static const UINT32 KEY_EXTRA_B  = (1UL << 25);
    static const UINT32 KEY_EXTRA_C  = (1UL << 26);
    static const UINT32 KEY_EXTRA_D  = (1UL << 27);
    static const UINT32 KEY_EXTRA_E  = (1UL << 28);
    static const UINT32 KEY_EXTRA_F  = (1UL << 29);
    static const UINT32 KEY_EXTRA_G  = (1UL << 30);
    static const UINT32 KEY_EXTRA_H  = (1UL << 31);

    // Keypad buttons for a device
    UINT32   m_nButtons;
    UINT32   m_nLastButtons;
    UINT32   m_nPressedButtons;

    // Pointer support for devices with pointers (mouse, touch, stylus, etc.)
    static const UINT32 POINTER_DOWN     = (1UL << 0);
    static const UINT32 POINTER_PRESSED  = (1UL << 1);
    static const UINT32 POINTER_RELEASED = (1UL << 2);

    UINT32     m_nPointerState;
    FRMVECTOR2 m_vPointerPosition;

	// Support for accelerometer and gyroscope
	BOOL	   m_AccelSupported;
	FRMVECTOR3 m_AccelRaw;		// Raw acceleration input
	FRMVECTOR3 m_AccelGravity;	// Gravity component
	FLOAT32	   m_AccelHPFK;		// High pass filter co-efficient
	FRMVECTOR3 m_AccelSmooth;	// Smoothed out acceleration value
	FRMVECTOR3 m_AccelVelocity;	// Integrated velocity value

	VOID ResetAccelerometer();

	BOOL	   m_GyroSupported;
	FRMVECTOR3 m_GyroRaw;
	FRMVECTOR3 m_GyroOffset;	// used to calibrate the raw input to reduce drift error
	FRMVECTOR3 m_GyroSmooth;

	VOID ResetGyroscope();

    VOID GetButtonState( UINT32* pnButtons, UINT32* pnPressedButtons );
    VOID GetPointerState( UINT32* pnPointerState, FRMVECTOR2* pvPointerPosition );
};


//--------------------------------------------------------------------------------------
// Name: FrmGetInput()
// Desc: Retrieves the current state of input
//--------------------------------------------------------------------------------------
VOID FrmGetInput( FRM_INPUT* pInput, UINT32* pnButtons, UINT32* pnPressedButtons );
VOID FrmGetMouseInput(FRM_INPUT *pInput, UINT32* pnPointerState, FRMVECTOR2 *pvPointerPosition);

#endif // _FRM_INPUT_H_
