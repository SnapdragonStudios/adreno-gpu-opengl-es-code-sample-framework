//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmPlatform.h"
#include "FrmUserInterface.h"
#include "FrmInput.h"


//--------------------------------------------------------------------------------------
// UserInterface variable types
//--------------------------------------------------------------------------------------
enum
{ 
    FRM_UI_VARIABLE_FLOAT = 0,
    FRM_UI_VARIABLE_STRING,
    FRM_UI_VARIABLE_INT,
    FRM_UI_VARIABLE_BOOL
};


//--------------------------------------------------------------------------------------
// The states that the UserInterface can be in
//--------------------------------------------------------------------------------------
enum
{ 
    FRM_UI_INACTIVE = 0,
    FRM_UI_DISPLAY_VARIABLES_STATE,
    FRM_UI_DISPLAY_HELP_KEYS_STATE,
    FRM_UI_MAX_STATE,
};


//--------------------------------------------------------------------------------------
// Name: CFrmUserInterface()
// Desc: CFrmUserInterface constructor
//--------------------------------------------------------------------------------------
CFrmUserInterface::CFrmUserInterface()
{
    m_pFont           = NULL;
    m_nState          = FRM_UI_INACTIVE,
    m_strHeading      = NULL;
    m_fHelpLeft       = 10.0f;
    m_fHelpTop        = 30.0f;

    m_nNumHelpKeys    = 0;
    m_nNumVariables   = 0;
    m_nNumTextStrings = 0;
    m_nNumOverlays    = 0;

    // Initialize memory
    FrmMemset( m_pTextStrings, 0, MAX_UI_TEXT_STRINGS );
    FrmMemset( m_pHelpKeys,    0, MAX_UI_HELP_KEYS );
    FrmMemset( m_pVariables,   0, MAX_UI_VARIABLES );
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmUserInterface()
// Desc: CFrmUserInterface destructor
//--------------------------------------------------------------------------------------
CFrmUserInterface::~CFrmUserInterface()
{
    FreeTextStringInfo();
    FreeHelpKeyInfo();
    FreeVariableInfo();
    FreeOverlayInfo();
}


//--------------------------------------------------------------------------------------
// Name: Initialize()
// Desc: Initialize the user interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::Initialize( CFrmFont* pFont, const CHAR* strHeading )
{
    m_pFont      = pFont;
    m_strHeading = strHeading;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: SetHeading()
// Desc: Adds sample text to the sample interface.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::SetHeading( const CHAR* strHeading )
{
    m_strHeading = strHeading;
}


//--------------------------------------------------------------------------------------
// Name: AddLogo()
// Desc: Adds a logo to the sample interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddOverlay( UINT32 hTextureHandle, FLOAT32 fX, FLOAT32 fY,
                                    UINT32 nWidth, UINT32 nHeight )
{
    if( m_nNumOverlays >= MAX_UI_OVERLAYS )
        return FALSE;

    FRM_UI_OVERLAY_INFO* pOverlayInfo = new FRM_UI_OVERLAY_INFO;
    if( pOverlayInfo == NULL )
        return FALSE;

    pOverlayInfo->m_hTextureHandle  = hTextureHandle;
    pOverlayInfo->m_fX              = fX;
    pOverlayInfo->m_fY              = fY;
    pOverlayInfo->m_fWidth          = (FLOAT32)nWidth;
    pOverlayInfo->m_fHeight         = (FLOAT32)nHeight;
    m_pOverlays[ m_nNumOverlays++ ] = pOverlayInfo;

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: AddLogo()
// Desc: Adds a logo to the sample interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddOverlay( VOID* pTexture, FLOAT32 fX, FLOAT32 fY,
                                    UINT32 nWidth, UINT32 nHeight )
{
    if( m_nNumOverlays >= MAX_UI_OVERLAYS )
        return FALSE;

    FRM_UI_OVERLAY_INFO* pOverlayInfo = new FRM_UI_OVERLAY_INFO;
    if( pOverlayInfo == NULL )
        return FALSE;

    pOverlayInfo->m_pTexture        = pTexture;
    pOverlayInfo->m_fX              = fX;
    pOverlayInfo->m_fY              = fY;
    pOverlayInfo->m_fWidth          = (FLOAT32)nWidth;
    pOverlayInfo->m_fHeight         = (FLOAT32)nHeight;
    m_pOverlays[ m_nNumOverlays++ ] = pOverlayInfo;

    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: AddTextString()
// Desc: Adds sample text to the sample interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddTextString( CHAR* strText, FLOAT32 fX, FLOAT32 fY, FLOAT32 fSize )
{
    if( m_nNumTextStrings >= MAX_UI_TEXT_STRINGS )
        return FALSE;

    // Check to make sure that our sample text string isn'ttoo large
    if( FrmStrlen( strText ) > MAX_UI_TEXT_STRING_TEXT_SIZE )
    {
        FrmLogMessage( "Text string: ", strText, " is too large" );
    }

    FRM_UI_TEXT_STRING_INFO* pTextInfo = new FRM_UI_TEXT_STRING_INFO;
    if( pTextInfo == NULL )
        return FALSE;

    FrmStrcpy( pTextInfo->m_strText, strText );
    pTextInfo->m_fX    = fX;
    pTextInfo->m_fY    = fY;
    pTextInfo->m_fSize = fSize;
    m_pTextStrings[ m_nNumTextStrings++ ] = pTextInfo;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: AddHelpKey()
// Desc: Adds a key and it's associated action to the help interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddHelpKey( CHAR* strKey, CHAR* strAction )
{
    if( m_nNumHelpKeys >= MAX_UI_HELP_KEYS )
        return FALSE;

    // Check to make sure that our key / action names are not too large
    if( FrmStrlen( strKey ) > MAX_UI_HELP_KEY_TEXT_SIZE )
    {
        FrmLogMessage( "Key string: ", strKey, " is too large" );
    }
    if( FrmStrlen( strAction ) > MAX_UI_HELP_KEY_ACTION_TEXT_SIZE )
    {
        FrmLogMessage( "Key action string: ", strAction, " is too large");
    }

    FRM_UI_HELP_KEY_INFO* pHelpKeyInfo = new FRM_UI_HELP_KEY_INFO;
    if( pHelpKeyInfo == NULL )
        return FALSE;

    FrmStrcpy( pHelpKeyInfo->m_strKey, strKey );
    FrmStrcpy( pHelpKeyInfo->m_strAction, strAction );
    m_pHelpKeys[ m_nNumHelpKeys++ ] = pHelpKeyInfo;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: AddFloatVariable()
// Desc: Adds a float variable to the help interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddFloatVariable( FLOAT32* pVariable, CHAR* strName, const CHAR* strFormat )
{
    return AddVariable( (VOID*)pVariable, FRM_UI_VARIABLE_FLOAT, strName, strFormat );
}


//--------------------------------------------------------------------------------------
// Name: AddStringVariable()
// Desc: Adds a string variable to the help interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddStringVariable( const CHAR** pVariable, CHAR* strName, const CHAR* strFormat )
{
    return AddVariable( (VOID*)pVariable, FRM_UI_VARIABLE_STRING, strName, strFormat );
}


//--------------------------------------------------------------------------------------
// Name: AddIntVariable()
// Desc: Adds an int variable to the help interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddIntVariable( INT32* pVariable, CHAR* strName, const CHAR* strFormat )
{
    return AddVariable( (VOID*)pVariable, FRM_UI_VARIABLE_INT, strName, strFormat );
}


//--------------------------------------------------------------------------------------
// Name: AddBoolVariable()
// Desc: Adds a bool variable to the help interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddBoolVariable( BOOL* pVariable, CHAR* strName, const CHAR* strTrue, const CHAR* strFalse )
{
    if( FALSE == AddVariable( (VOID*) pVariable, FRM_UI_VARIABLE_BOOL, strName, "%s" ) )
        return FALSE;

    if( FrmStrlen( strTrue ) > MAX_UI_VARIABLE_BOOL_TEXT_SIZE )
    {
        FrmLogMessage( "Variable: ", strName, " true string is too large" );
        return FALSE;
    }

    if( FrmStrlen( strFalse ) > MAX_UI_VARIABLE_BOOL_TEXT_SIZE )
    {
        FrmLogMessage( "Variable: ", strName, " false string is too large" );
        return FALSE;
    }

    FrmStrcpy( m_pVariables[ m_nNumVariables - 1 ]->m_strBoolTrue, strTrue );
    FrmStrcpy( m_pVariables[ m_nNumVariables - 1 ]->m_strBoolFalse, strFalse );

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: AdvanceState()
// Desc: Advances the state of the UI
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::AdvanceState()
{
    m_nState = ( m_nState + 1 ) % FRM_UI_MAX_STATE;
}


//--------------------------------------------------------------------------------------
// Name: HandleInput()
// Desc: Convenient helper to handle application input
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::HandleInput( UINT32 nWidth, UINT32 nHeight, UINT32 nPressedButtons,
                                     UINT32 nPointerState, FRMVECTOR2 vPointerPosition )
{
    // Advance the UI state when the user presses '0'
    if( nPressedButtons & FRM_INPUT::KEY_0 )
    {
        AdvanceState();
    }
    // Conditionally advance the UI state when the user clicks the pointer
    else if( nPointerState & FRM_INPUT::POINTER_PRESSED )
    {
        // When the UI is active, any click will advance the state.
        if( IsActive() )
        {
            AdvanceState();
        }
        // Otherwise, if the user clicked below the help line, display the UI
        else if( nHeight * ( vPointerPosition.y * 0.5f + 0.5f ) < 30 )
        {
            AdvanceState();
        }
    }
}

//--------------------------------------------------------------------------------------
// Name: Render()
// Desc: Renders the user interface
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::Render( FLOAT32 fFrameRate )
{
    if( m_nState == FRM_UI_DISPLAY_VARIABLES_STATE )
    {
        if( m_nNumVariables == 0 )
        {
            // If there's nothing to do, advance out of this state
            AdvanceState();
        }
        else
        {
            // Display the variables
            RenderFadedBackground();
            RenderVariables();
        }
    }

    if( m_nState == FRM_UI_DISPLAY_HELP_KEYS_STATE )
    {
        if( m_nNumHelpKeys == 0 )
        {
            // If there's nothing to do, advance out of this state
            AdvanceState();
        }
        else
        {
            // Display the keys
            RenderFadedBackground();
            RenderHelpKeys();
        }
    }

    if( m_nState == FRM_UI_INACTIVE )
    {
        RenderTextStrings();

        if ((fFrameRate > 0.0f) && (m_pFont))
        {
            CHAR strFrameRate[40];
            FrmSprintf( strFrameRate, 40, "%0.1f fps", fFrameRate );
            m_pFont->SetScaleFactors( 0.8f, 0.8f );
            m_pFont->DrawText( -1, 1, FRMCOLOR_WHITE, strFrameRate, FRM_FONT_RIGHT );
        }
    }

    // Display the title
    RenderTitle();

    // Display overlays
    RenderOverlays();
}


//--------------------------------------------------------------------------------------
// Name: AddVariable()
// Desc: Adds a variable to the user interface.
//--------------------------------------------------------------------------------------
BOOL CFrmUserInterface::AddVariable( VOID* pVariable, UINT32 nType, CHAR* strName,
                                     const CHAR* strFormat )
{
    if( m_nNumVariables >= MAX_UI_VARIABLES )
        return FALSE;

    // Check to make sure that our name is not too large
    if( FrmStrlen( strName ) > MAX_UI_VARIABLE_NAME_TEXT_SIZE )
    {
        FrmLogMessage( "Variable name: ", strName, " is too large" );
    }

    // Check to make sure that our format is not too large
    if( FrmStrlen( strFormat ) > MAX_UI_VARIABLE_FORMAT_TEXT_SIZE )
    {
        FrmLogMessage( "Variable format: ", strFormat, " is too large" );
    }

    FRM_UI_VARIABLE_INFO* pVariableInfo = new FRM_UI_VARIABLE_INFO;
    if( pVariableInfo == NULL )
        return FALSE;

    pVariableInfo->m_pVar = pVariable;
    pVariableInfo->m_nType = nType;
    FrmStrcpy( pVariableInfo->m_strName, strName );
    FrmStrcpy( pVariableInfo->m_strFormat, strFormat );
    m_pVariables[ m_nNumVariables++ ] = pVariableInfo;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: RenderTextStrings()
// Desc: Renders the sample text
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::RenderTextStrings()
{
    if (m_pFont != NULL)
    {
        // Display sample text
        m_pFont->Begin();
        for( UINT32 nTextIndex = 0; nTextIndex < m_nNumTextStrings; ++nTextIndex )
        {
            FRM_UI_TEXT_STRING_INFO* pTextInfo = m_pTextStrings[ nTextIndex ];
            m_pFont->SetScaleFactors( pTextInfo->m_fSize, pTextInfo->m_fSize );
            m_pFont->DrawText( pTextInfo->m_fX, pTextInfo->m_fY, FRMCOLOR_WHITE, 
                               pTextInfo->m_strText );
        }
        m_pFont->SetScaleFactors( 1.0f, 1.0f );
        m_pFont->End();
    }
}


//--------------------------------------------------------------------------------------
// Name: RenderVariables()
// Desc: Renders the help interface's variable information
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::RenderVariables()
{
    if (!m_pFont)
        return;

    FLOAT32 fTextHeight = m_pFont->GetTextHeight() - 2.0f;
    m_pFont->Begin();
    m_pFont->SetScaleFactors( 1.0f, 1.0f );

    // Determine the column positions
    FLOAT32 fColumn1Left = m_fHelpLeft;
    FLOAT32 fColumn2Left = m_fHelpLeft + m_pFont->GetTextWidth( "Variable" );

    m_pFont->SetScaleFactors( 0.8f, 0.8f );
    for( UINT32 nVariableIndex = 0; nVariableIndex < m_nNumVariables; ++nVariableIndex )
    {
        FRM_UI_VARIABLE_INFO* pVariableInfo = m_pVariables[ nVariableIndex ];
        if( pVariableInfo == NULL ) continue;

        FLOAT32 fTextWidth = m_pFont->GetTextWidth( pVariableInfo->m_strName );
        if( m_fHelpLeft + fTextWidth > fColumn2Left )
            fColumn2Left = m_fHelpLeft + fTextWidth;
    }
    fColumn2Left += m_pFont->GetTextWidth( "     " );

    m_pFont->SetScaleFactors( 1.0f, 1.0f );
    m_pFont->DrawText( fColumn1Left, m_fHelpTop, FRMCOLOR_YELLOW, "Variable" );
    m_pFont->DrawText( fColumn2Left, m_fHelpTop, FRMCOLOR_YELLOW, "Value" );
    FLOAT32 fTextTop = m_fHelpTop + 1.3f * fTextHeight;
            
    // Render the variable names and values
    m_pFont->SetScaleFactors( 0.8f, 0.8f );
    for( UINT32 nVariableIndex = 0; nVariableIndex < m_nNumVariables; ++nVariableIndex )
    {
        FRM_UI_VARIABLE_INFO* pVariableInfo = m_pVariables[ nVariableIndex ];
        if( pVariableInfo == NULL ) continue;

        // Format a string to display the variable value
        const UINT32 nVarMaxSize = MAX_UI_VARIABLE_VALUE_TEXT_SIZE + 1;
        CHAR strFormatedVariable[ nVarMaxSize ];
        switch( pVariableInfo->m_nType )
        {
            case FRM_UI_VARIABLE_FLOAT:
            {
                FLOAT32* pVar = (FLOAT32*)pVariableInfo->m_pVar;
                FrmSprintf( strFormatedVariable, nVarMaxSize, pVariableInfo->m_strFormat, *pVar );
                break;
            }
            case FRM_UI_VARIABLE_STRING:
            {
                CHAR** pVar = (CHAR**)pVariableInfo->m_pVar;
                if( *pVar != NULL )
                    FrmSprintf( strFormatedVariable, nVarMaxSize, pVariableInfo->m_strFormat, *pVar );
                break;
            }
            case FRM_UI_VARIABLE_INT:
            {
                INT32* pVar = (INT32*)pVariableInfo->m_pVar;
                FrmSprintf( strFormatedVariable, nVarMaxSize, pVariableInfo->m_strFormat, *pVar );
                break;
            }
            case FRM_UI_VARIABLE_BOOL:
            {
                BOOL* pVar = (BOOL*)pVariableInfo->m_pVar;
                FrmSprintf( strFormatedVariable, nVarMaxSize, pVariableInfo->m_strFormat,
                            *pVar ? pVariableInfo->m_strBoolTrue : pVariableInfo->m_strBoolFalse );
                break;
            }
        }

        m_pFont->DrawText( fColumn1Left, fTextTop, FRMCOLOR_YELLOW, pVariableInfo->m_strName );
        m_pFont->DrawText( fColumn2Left, fTextTop, FRMCOLOR_YELLOW, strFormatedVariable );
        fTextTop += fTextHeight;
    }

    m_pFont->SetScaleFactors( 1.0f, 1.0f );
    m_pFont->End();
}


//--------------------------------------------------------------------------------------
// Name: RenderHelpKeys()
// Desc: Renders the help interface's key information
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::RenderHelpKeys()
{
    if (!m_pFont)
        return;

    FLOAT32 fTextHeight = m_pFont->GetTextHeight() - 2.0f;
    m_pFont->Begin();
    m_pFont->SetScaleFactors( 1.0f, 1.0f );

    FLOAT32 fColumn1Left = m_fHelpLeft;
    FLOAT32 fColumn2Left = m_fHelpLeft + m_pFont->GetTextWidth( "Key     " );

    m_pFont->DrawText( fColumn1Left, m_fHelpTop, FRMCOLOR_GREEN, "Key" );
    m_pFont->DrawText( fColumn2Left, m_fHelpTop, FRMCOLOR_GREEN, "Action" );
    FLOAT32 fTextTop = m_fHelpTop + 1.3f * fTextHeight;

    m_pFont->SetScaleFactors( 0.8f, 0.8f );
    for( UINT32 nKeyIndex = 0; nKeyIndex < m_nNumHelpKeys; ++nKeyIndex )
    {
        FRM_UI_HELP_KEY_INFO* pKeyInfo = m_pHelpKeys[ nKeyIndex ];
        if( pKeyInfo == NULL ) continue;

        m_pFont->DrawText( fColumn1Left, fTextTop, FRMCOLOR_GREEN, pKeyInfo->m_strKey );
        m_pFont->DrawText( fColumn2Left, fTextTop, FRMCOLOR_GREEN, pKeyInfo->m_strAction );
        fTextTop += fTextHeight;
    }
    m_pFont->SetScaleFactors( 1.0f, 1.0f );
    m_pFont->End();
}


//--------------------------------------------------------------------------------------
// Name: RenderTitle()
// Desc: Renders the sample title
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::RenderTitle()
{
    if ((m_strHeading) && (m_pFont))
    {
        m_pFont->SetScaleFactors( 1.2f, 1.2f );
        m_pFont->DrawText( 1.0f, 1.0f, FRMCOLOR_WHITE, m_strHeading );
        m_pFont->SetScaleFactors( 1.0f, 1.0f );
    }
}


//--------------------------------------------------------------------------------------
// Name: FreeTextStringInfo()
// Desc: Frees the memory used by the sample text information.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::FreeTextStringInfo()
{
    for( UINT32 nTextStringIndex = 0; nTextStringIndex < m_nNumTextStrings; ++nTextStringIndex )
    {
        if( m_pTextStrings[ nTextStringIndex ] == NULL )
            break;

        delete m_pTextStrings[ nTextStringIndex ];
    }

    m_nNumTextStrings = 0;
}


//--------------------------------------------------------------------------------------
// Name: FreeHelpKeyInfo()
// Desc: Frees the memory used by the key information.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::FreeHelpKeyInfo()
{
    for( UINT32 nKeyIndex = 0; nKeyIndex < m_nNumHelpKeys; ++nKeyIndex )
    {
        if( m_pHelpKeys[ nKeyIndex ] == NULL )
            break;

        delete m_pHelpKeys[ nKeyIndex ];
    }

    m_nNumHelpKeys = 0;
}


//--------------------------------------------------------------------------------------
// Name: FreeVariableInfo()
// Desc: Frees the memory used by the variable information.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::FreeVariableInfo()
{
    for( UINT32 nVariableIndex = 0; nVariableIndex < m_nNumVariables; ++nVariableIndex )
    {
        if( m_pVariables[ nVariableIndex ] == NULL )
            break;

        delete m_pVariables[ nVariableIndex ];
    }

    m_nNumVariables = 0;
}


//--------------------------------------------------------------------------------------
// Name: FreeOverlayInfo()
// Desc: Frees the memory used by overlays.
//--------------------------------------------------------------------------------------
VOID CFrmUserInterface::FreeOverlayInfo()
{
    for( UINT32 i = 0; i < m_nNumOverlays; ++i )
        delete m_pOverlays[i];
    m_nNumOverlays = 0;
}


