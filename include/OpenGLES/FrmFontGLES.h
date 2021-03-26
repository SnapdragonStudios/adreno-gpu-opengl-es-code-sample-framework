// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_FONT_GLES_H
#define _FRM_FONT_GLES_H

#include "FrmFont.h"
#include "OpenGLES/FrmPackedResourceGLES.h"

//#include "Crossplatform.h"

#if LINUX_OR_OSX && !defined(ANDROID)
#include <stdio.h>
#include <netinet/in.h>
//#elif defined(_WIN32) || defined(_WIN64)
//#include <Winsock2.h>
#endif

//--------------------------------------------------------------------------------------
// Name: struct FRM_FONT_GLYPH_GLES
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_FONT_GLYPH_GLES
{
    UINT16 u0, v0;
    UINT16 u1, v1;
    INT16  nOffset;
    INT16  nWidth;
    INT16  nAdvance;
    void ToHostEndian()
    {
      u0= ntohs(u0);
      v0= ntohs(v0);
      u1= ntohs(u1);
      v1= ntohs(v1);
      nWidth= ntohs(nWidth);
      nAdvance= ntohs(nAdvance);
    }
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_FONT_FILE_HEADER_GLES
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_FONT_FILE_HEADER_GLES
{
    UINT32 nMagicID;
    UINT32 nGlyphHeight;
    FRM_FONT_GLYPH_GLES pGlyphs[1];

    void ToHostEndian()
    {
      nMagicID= ntohl(nMagicID);
      nGlyphHeight= ntohl(nGlyphHeight);
      pGlyphs[0].ToHostEndian();
    }
};


//--------------------------------------------------------------------------------------
// Name: class CFrmFont
// Desc: 
//--------------------------------------------------------------------------------------
class CFrmFontGLES : public CFrmFont
{
public:
    CFrmFontGLES();
    virtual ~CFrmFontGLES();

    // Create/Destroy functions
    virtual BOOL    Create( const CHAR* strFontFileName );
    virtual VOID    Destroy();

    // Text helper functions
    virtual VOID    GetTextExtent( const CHAR* strText, FLOAT32* pSX, FLOAT32* pSY );
    virtual FLOAT32 GetTextWidth( const CHAR* strText );
    virtual FLOAT32 GetTextHeight();
    virtual VOID    SetScaleFactors( FLOAT32 fScaleX, FLOAT32 fScaleY );

    // Text drawing functions
    virtual VOID    Begin();
    virtual VOID    DrawText( FLOAT32 sx, FLOAT32 sy, FRMCOLOR nColor, 
                              const CHAR* strText, UINT32 nFlags = 0 );
    virtual VOID    End();

protected:
    FRM_FONT_GLYPH_GLES*    m_pGlyphsGLES;

    CFrmPackedResourceGLES  m_ResourceGLES;
    CFrmTexture*            m_pTexture;

    FLOAT32                 m_fScaleX;
    FLOAT32                 m_fScaleY;

    UINT32                  m_hShader;
    UINT32                  m_locVertexPos;
    UINT32                  m_locVertexColor;
    UINT32                  m_locVertexTex;
    UINT32                  m_locScale;
    UINT32                  m_locOffset;

    UINT32                  m_nNestedBeginCount;
};


#endif // _FRM_FONT_GLES_H
