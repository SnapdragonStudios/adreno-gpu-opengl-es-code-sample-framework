//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _FRM_USER_INTERFACE_H_
#define _FRM_USER_INTERFACE_H_

#include "FrmPlatform.h"
#include "FrmFont.h"


// Interface related constants
const UINT32 MAX_UI_OVERLAYS                  =  16;

const UINT32 MAX_UI_TEXT_STRINGS              =  16;
const UINT32 MAX_UI_TEXT_STRING_TEXT_SIZE     = 128;

const UINT32 MAX_UI_HELP_KEYS                 =  16;
const UINT32 MAX_UI_HELP_KEY_TEXT_SIZE        =   4;
const UINT32 MAX_UI_HELP_KEY_ACTION_TEXT_SIZE = 128;

const UINT32 MAX_UI_VARIABLES                 =  16;
const UINT32 MAX_UI_VARIABLE_NAME_TEXT_SIZE   =  32;
const UINT32 MAX_UI_VARIABLE_FORMAT_TEXT_SIZE =  16;
const UINT32 MAX_UI_VARIABLE_VALUE_TEXT_SIZE  =  32;
const UINT32 MAX_UI_VARIABLE_BOOL_TEXT_SIZE   =  16;


//--------------------------------------------------------------------------------------
// Name: struct FRM_UI_OVERLAY_INFO
// Desc: Info for rendering a user-supplied overlay
//--------------------------------------------------------------------------------------
struct FRM_UI_OVERLAY_INFO
{
    union 
    {
        UINT32    m_hTextureHandle;
        VOID*     m_pTexture; // Needed for D3D which does not store textures as UINT32
    };
    FLOAT32   m_fX;
    FLOAT32   m_fY;
    FLOAT32   m_fWidth;
    FLOAT32   m_fHeight;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_UI_TEXT_STRING_INFO
// Desc: Info for rendering a user-supplied text string
//--------------------------------------------------------------------------------------
struct FRM_UI_TEXT_STRING_INFO
{
    CHAR      m_strText[ MAX_UI_TEXT_STRING_TEXT_SIZE ];
    FLOAT32   m_fX;
    FLOAT32   m_fY;
    FLOAT32   m_fSize;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_VARIABLE_INFO
// Desc: Info for rendering a user-supplied variable
//--------------------------------------------------------------------------------------
struct FRM_UI_VARIABLE_INFO
{
    VOID*  m_pVar;
    UINT32 m_nType;
    CHAR   m_strName[ MAX_UI_VARIABLE_NAME_TEXT_SIZE ];
    CHAR   m_strFormat[ MAX_UI_VARIABLE_FORMAT_TEXT_SIZE ];

    // Only needed for bools
    CHAR   m_strBoolTrue[ MAX_UI_VARIABLE_BOOL_TEXT_SIZE ];
    CHAR   m_strBoolFalse[ MAX_UI_VARIABLE_BOOL_TEXT_SIZE ];
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_UI_HELP_KEY_INFO
// Desc: Info for rendering help on a specific key
//--------------------------------------------------------------------------------------
struct FRM_UI_HELP_KEY_INFO
{
    CHAR m_strKey[ MAX_UI_HELP_KEY_TEXT_SIZE ];
    CHAR m_strAction[ MAX_UI_HELP_KEY_ACTION_TEXT_SIZE ];
};


//--------------------------------------------------------------------------------------
// Name: class CFrmUserInterface
// Desc: Represents a sample's user interface components.
//--------------------------------------------------------------------------------------
class CFrmUserInterface
{
public:
    CFrmUserInterface();
    virtual ~CFrmUserInterface();

    // Initialize the user interface
    virtual BOOL Initialize( CFrmFont* pFont, const CHAR* strHeading );
    VOID SetHeading( const CHAR* strHeading );

    // Add items to the user interface
    BOOL AddOverlay( UINT32 hTextureHandle, FLOAT32 fX, FLOAT32 fY, UINT32 nWidth, UINT32 nHeight );
    BOOL AddOverlay( VOID* hTextureHandle, FLOAT32 fX, FLOAT32 fY, UINT32 nWidth, UINT32 nHeight );
    BOOL AddTextString( CHAR* strText, FLOAT32 fX, FLOAT32 fY, FLOAT32 fSize = 0.8f );
    BOOL AddHelpKey( CHAR* strKey, CHAR* strAction );
    BOOL AddFloatVariable( FLOAT32* pVariable, CHAR* strName, const CHAR* strFormat = "%4.2f" );
    BOOL AddStringVariable( const CHAR** pVariable, CHAR* strName, const CHAR* strFormat = "%s" );
    BOOL AddIntVariable( INT32* pVariable, CHAR* strName, const CHAR* strFormat = "%d" );
    BOOL AddBoolVariable( BOOL* pVariable, CHAR* strName, const CHAR* strTrue = "True", const CHAR* strFalse = "False" );

    // Handle which state the user interface is in
    VOID AdvanceState();
    BOOL IsActive() { return ( m_nState > 0 ) ? TRUE : FALSE; }

    // Render the user interface
    VOID Render( FLOAT32 fFrameRate = 0.0f );

    // Convenient helper to handle application input
    VOID HandleInput( UINT32 nWidth, UINT32 nHeight, UINT32 nPressedButtons,
                      UINT32 nPointerState, FRMVECTOR2 vPointerPosition );

protected:
    BOOL AddVariable( VOID* pVariable, UINT32 nType, CHAR* strName, const CHAR* strFormat );

    // Render functions
    virtual VOID RenderFadedBackground() = 0;
    virtual VOID RenderOverlays() = 0;
    VOID RenderVariables();
    VOID RenderHelpKeys();
    VOID RenderTextStrings();
    VOID RenderTitle();

    // Free alocated memory
    VOID FreeTextStringInfo();
    VOID FreeHelpKeyInfo();
    VOID FreeVariableInfo();
    VOID FreeOverlayInfo();

    // All state variables
    CFrmFont*                m_pFont;
    UINT32                   m_nState;  // The currently displayed state
    const CHAR*              m_strHeading;

    // Help state variables

    // Top left corner of the help screen
    FLOAT32                  m_fHelpLeft;
    FLOAT32                  m_fHelpTop;

    // Help interface variables
    FRM_UI_VARIABLE_INFO*    m_pVariables[ MAX_UI_VARIABLES ];
    UINT32                   m_nNumVariables;

    FRM_UI_HELP_KEY_INFO*    m_pHelpKeys[ MAX_UI_HELP_KEYS ];
    UINT32                   m_nNumHelpKeys;

    FRM_UI_TEXT_STRING_INFO* m_pTextStrings[ MAX_UI_TEXT_STRINGS ];
    UINT32                   m_nNumTextStrings;

    FRM_UI_OVERLAY_INFO*     m_pOverlays[ MAX_UI_OVERLAYS ];
    UINT32                   m_nNumOverlays;
};


#endif // _FRM_USER_INTERFACE_H_

