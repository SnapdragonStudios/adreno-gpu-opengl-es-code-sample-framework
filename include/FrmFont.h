// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_FONT_H
#define _FRM_FONT_H

#include "FrmPlatform.h"
#include "FrmMath.h"
#include "FrmPackedResource.h"
#include "FrmUtils.h"


//--------------------------------------------------------------------------------------
// Magic ID to identify font files
//--------------------------------------------------------------------------------------
const UINT32 FRM_FONT_FILE_MAGIC_ID = ('F'<<0)|('O'<<8)|('N'<<16)|('T'<<24);


//--------------------------------------------------------------------------------------
// Flags for drawing text
//--------------------------------------------------------------------------------------
const UINT32 FRM_FONT_RIGHT  = (1L<<0L);
const UINT32 FRM_FONT_CENTER = (2L<<0L);


//--------------------------------------------------------------------------------------
// Name: class CFrmFont
// Desc: 
//--------------------------------------------------------------------------------------
class CFrmFont
{
public:
    CFrmFont();
    virtual ~CFrmFont();

    // Create/Destroy functions
    virtual BOOL    Create( const CHAR* strFontFileName ) = 0;
    virtual VOID    Destroy() = 0;

    // Text helper functions
    virtual VOID    GetTextExtent( const CHAR* strText, FLOAT32* pSX, FLOAT32* pSY ) = 0;
    virtual FLOAT32 GetTextWidth( const CHAR* strText ) = 0;
    virtual FLOAT32 GetTextHeight() = 0;
    virtual VOID    SetScaleFactors( FLOAT32 fScaleX, FLOAT32 fScaleY ) = 0;

    // Text drawing functions
    virtual VOID    Begin() = 0;
    virtual VOID    DrawText( FRMCOLOR nColor, const CHAR* strText, UINT32 nFlags = 0 );
    virtual VOID    DrawText( FLOAT32 sx, FLOAT32 sy, FRMCOLOR nColor, 
                              const CHAR* strText, UINT32 nFlags = 0 ) = 0;
    virtual VOID    End() = 0;

protected:
    UINT32             m_nGlyphHeight;

    UINT32             m_nViewportWidth;
    UINT32             m_nViewportHeight;

    FLOAT32            m_fCursorX;
    FLOAT32            m_fCursorY;
};


//--------------------------------------------------------------------------------------
// Glpyh assignments for fonts that optionally include custom-colored glyphs
//--------------------------------------------------------------------------------------
#define FRM_FONT_KEY_0         "\200"
#define FRM_FONT_KEY_1         "\201"
#define FRM_FONT_KEY_2         "\202"
#define FRM_FONT_KEY_3         "\203"
#define FRM_FONT_KEY_4         "\204"
#define FRM_FONT_KEY_5         "\205"
#define FRM_FONT_KEY_6         "\206"
#define FRM_FONT_KEY_7         "\207"
#define FRM_FONT_KEY_8         "\210"
#define FRM_FONT_KEY_9         "\211"
#define FRM_FONT_KEY_STAR      "\212"
#define FRM_FONT_KEY_HASH      "\213"
#define FRM_FONT_KEY_OK        "\214"
#define FRM_FONT_KEY_BACK      "\215"
#define FRM_FONT_KEY_UP        "\216"
#define FRM_FONT_KEY_DOWN      "\217"
#define FRM_FONT_KEY_LEFT      "\220"
#define FRM_FONT_KEY_RIGHT     "\221"
#define FRM_FONT_KEY_DPAD      "\222"
#define FRM_FONT_KEY_UPDOWN    "\223"
#define FRM_FONT_KEY_LEFTRIGHT "\224"


#endif // _FRM_FONT_H
