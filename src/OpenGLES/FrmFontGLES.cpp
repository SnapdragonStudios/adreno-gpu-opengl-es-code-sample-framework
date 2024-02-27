//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "FrmPlatform.h"
#include "OpenGLES/FrmFontGLES.h"
#include "OpenGLES/FrmShader.h"
//#include "Crossplatform.h"
#include <stdio.h>

#ifdef CORE_GL_CONTEXT
const CHAR* g_strFontVertexShader =
"#version 300 es\n"
"uniform   vec4 g_vScale;                                                \n"
"uniform   vec4 g_vOffset;                                               \n"
"in vec2 g_vVertexPos;                                            \n"
"in vec4 g_vVertexColor;                                          \n"
"in vec2 g_vVertexTex;                                            \n"
"out   vec4 g_vColor;                                                \n"
"out   vec2 g_vTexCoord;                                             \n"
"                                                                        \n"
"void main()                                                             \n"
"{                                                                       \n"
"    // Transform the position                                           \n"
"    vec2 vVertex   = g_vVertexPos * g_vScale.xy + g_vOffset.xy;         \n"
"    gl_Position = vec4( vVertex, 0.0, 1.0 );                            \n"
"                                                                        \n"
"    // Pass through color and texture coordinates                       \n"
"    g_vColor    = g_vVertexColor;                                       \n"
"    g_vTexCoord = g_vVertexTex * g_vScale.zw + g_vOffset.zw;            \n"
"}                                                                       \n";


const CHAR* g_strFontFragmentShader =
"#version 300 es\n"
"// OpenGL ES requires that precision is defined for a fragment shader   \n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH                                       \n"
"   precision highp float;                                               \n"
"#else                                                                   \n"
"   precision mediump float;                                             \n"
"#endif                                                                  \n"
"                                                                        \n"
"uniform sampler2D g_Texture;                                            \n"
"in vec4      g_vColor;                                                 \n"
"in vec2      g_vTexCoord;                                              \n"
"out vec4      g_FragColor;                                             \n"
"void main()                                                             \n"
"{                                                                       \n"
"    g_FragColor = g_vColor * texture( g_Texture, g_vTexCoord );      \n"
"}                                                                       \n";
#else
const CHAR* g_strFontVertexShader =
"uniform   vec4 g_vScale;                                                \n"
"uniform   vec4 g_vOffset;                                               \n"
"attribute vec2 g_vVertexPos;                                            \n"
"attribute vec4 g_vVertexColor;                                          \n"
"attribute vec2 g_vVertexTex;                                            \n"
"varying   vec4 g_vColor;                                                \n"
"varying   vec2 g_vTexCoord;                                             \n"
"                                                                        \n"
"void main()                                                             \n"
"{                                                                       \n"
"    // Transform the position                                           \n"
"    vec2 vVertex   = g_vVertexPos * g_vScale.xy + g_vOffset.xy;         \n"
"    gl_Position = vec4( vVertex, 0.0, 1.0 );                            \n"
"                                                                        \n"
"    // Pass through color and texture coordinates                       \n"
"    g_vColor    = g_vVertexColor;                                       \n"
"    g_vTexCoord = g_vVertexTex * g_vScale.zw + g_vOffset.zw;            \n"
"}                                                                       \n";


const CHAR* g_strFontFragmentShader =
"// OpenGL ES requires that precision is defined for a fragment shader   \n"
"#ifdef GL_FRAGMENT_PRECISION_HIGH                                       \n"
"   precision highp float;                                               \n"
"#else                                                                   \n"
"   precision mediump float;                                             \n"
"#endif                                                                  \n"
"                                                                        \n"
"uniform sampler2D g_Texture;                                            \n"
"varying vec4      g_vColor;                                             \n"
"varying vec2      g_vTexCoord;                                          \n"
"                                                                        \n"
"void main()                                                             \n"
"{                                                                       \n"
"    gl_FragColor = g_vColor * texture2D( g_Texture, g_vTexCoord );      \n"
"}\n";
#endif

//--------------------------------------------------------------------------------------
// Name: CFrmFontGLES()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFontGLES::CFrmFontGLES() : 
    CFrmFont()
{
    m_pGlyphsGLES       = NULL;
    m_pTexture          = NULL;

    m_fScaleX           = 1.0f;
    m_fScaleY           = 1.0f;

    m_hShader           = 0;
    m_locVertexPos      = 0;
    m_locVertexColor    = 0;
    m_locVertexTex      = 0;
    m_locScale          = 0;
    m_locOffset         = 0;

    m_nNestedBeginCount = 0;
}


//--------------------------------------------------------------------------------------
// Name: ~CFrmFontGLES()
// Desc: 
//--------------------------------------------------------------------------------------
CFrmFontGLES::~CFrmFontGLES()
{
    Destroy();
}


//--------------------------------------------------------------------------------------
// Name: Create()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmFontGLES::Create( const CHAR* strFontFileName )
{
    // Load the packed resources
    if( FALSE == m_ResourceGLES.LoadFromFile( strFontFileName ) )
    {
		FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - FALSE == m_ResourceGLES.LoadFromFile( strFontFileName )");
        return FALSE;
    }

    // Access the font data
    FRM_FONT_FILE_HEADER_GLES* pFontData;
    pFontData = (FRM_FONT_FILE_HEADER_GLES*)m_ResourceGLES.GetData( "FontData" );
    if( NULL == pFontData )
    {
		FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - if( NULL == pFontData )");
        return FALSE;
    }


    // Verify the font data is valid
    if( pFontData->nMagicID != FRM_FONT_FILE_MAGIC_ID )
    {
#if LINUX_OR_OSX && !defined(ANDROID)
        // If little endian...
        if(false==IsBigEndian())
        {
            pFontData->ToHostEndian();

            // If magicID still not correct after byte reversal...
            if( pFontData->nMagicID != FRM_FONT_FILE_MAGIC_ID )
            {
				FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - if( pFontData->nMagicID != FRM_FONT_FILE_MAGIC_ID )");
                return FALSE;
            }
        }
        else
#endif
        {
			FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - if( pFontData->nMagicID != FRM_FONT_FILE_MAGIC_ID )");
            return FALSE;
        }
    }

    // Extract the glyphs
    m_nGlyphHeight = pFontData->nGlyphHeight;
    m_pGlyphsGLES  = pFontData->pGlyphs;

    // Create the font texture
    m_pTexture = m_ResourceGLES.GetTexture( "FontTexture" );
    if( NULL == m_pTexture )
    {
		FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - if( NULL == m_pTexture )");
         return FALSE;
    }

    // Compile the font shaders
    if( FALSE == FrmCompileShaderProgram( g_strFontVertexShader, g_strFontFragmentShader, &m_hShader ) )
    {
		FrmLogMessage( "ERROR: CFrmFontGLES::Create( const CHAR* strFontFileName ) failed.  - if( FALSE == FrmCompileShaderProgram( g_strFontVertexShader, g_strFontFragmentShader, &m_hShader ) )");
         return FALSE;
    }

    m_locVertexPos   = glGetAttribLocation( m_hShader, "g_vVertexPos" );
    m_locVertexColor = glGetAttribLocation( m_hShader, "g_vVertexColor" );
    m_locVertexTex   = glGetAttribLocation( m_hShader, "g_vVertexTex" );

    m_locScale       = glGetUniformLocation( m_hShader, "g_vScale" );
    m_locOffset      = glGetUniformLocation( m_hShader, "g_vOffset" );

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::Destroy()
{
    if( m_hShader ) glDeleteProgram( m_hShader );
    m_hShader = static_cast<UINT32>(NULL);

    if( m_pTexture ) m_pTexture->Release();
    m_pTexture = NULL;

    m_ResourceGLES.Destroy();
}


//--------------------------------------------------------------------------------------
// Name: GetTextExtent()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::GetTextExtent( const CHAR* strText, FLOAT32* pSX, FLOAT32* pSY )
{
    INT32 sx = 0;
    INT32 sy = m_nGlyphHeight;

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
        {
            sx  = 0;
            sy += m_nGlyphHeight;
            continue;
        }

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += pGlyph->nOffset;
        sx += pGlyph->nAdvance;
    }

    if(pSX) (*pSX) = m_fScaleX * sx;
    if(pSY) (*pSY) = m_fScaleY * sy;
}


//--------------------------------------------------------------------------------------
// Name: GetTextWidth()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 CFrmFontGLES::GetTextWidth( const CHAR* strText )
{
    INT32 sx = 0;

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
            break;

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += pGlyph->nOffset;
        sx += pGlyph->nAdvance;
    }

    return m_fScaleX * sx;
}


//--------------------------------------------------------------------------------------
// Name: GetTextHeight()
// Desc: 
//--------------------------------------------------------------------------------------
FLOAT32 CFrmFontGLES::GetTextHeight()
{
    return m_fScaleY * m_nGlyphHeight;
}


//--------------------------------------------------------------------------------------
// Name: SetScaleFactors()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::SetScaleFactors( FLOAT32 fScaleX, FLOAT32 fScaleY )
{
    m_fScaleX = fScaleX;
    m_fScaleY = fScaleY;
}


//--------------------------------------------------------------------------------------
// Name: Begin()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::Begin()
{
    if( 0 == m_nNestedBeginCount++ )
    {
        struct { GLint x, y, nWidth, nHeight; } Viewport;
        glGetIntegerv( GL_VIEWPORT, (GLint*)&Viewport );
        m_nViewportWidth  = Viewport.nWidth;
        m_nViewportHeight = Viewport.nHeight;

        FLOAT32 fTextureWidth  = (FLOAT32)m_pTexture->m_nWidth;
        FLOAT32 fTextureHeight = (FLOAT32)m_pTexture->m_nHeight;

        // Set the texture
        glActiveTexture( GL_TEXTURE0 );
        glBindTexture( GL_TEXTURE_2D, m_pTexture->m_hTextureHandle );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
        glEnable( GL_BLEND );

        // Set render states
        glDisable( GL_DEPTH_TEST );

        // Set the shader program
        glUseProgram( m_hShader );
        glUniform4f( m_locScale, +2.0f / m_nViewportWidth, -2.0f / m_nViewportHeight, 
                                 1.0f/fTextureWidth, -1.0f/fTextureHeight );
        glUniform4f( m_locOffset, -1.0f, +1.0f, 0.0f, 1.0f );
    }
}


//--------------------------------------------------------------------------------------
// Name: DrawText()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::DrawText( FLOAT32 sx, FLOAT32 sy, FRMCOLOR nColor, const CHAR* strText,
                         UINT32 nFlags )
{
    Begin();

    sx = FrmFloor( sx );
    sy = FrmFloor( sy );

    if( sx < 0.0f ) sx += (FLOAT32)m_nViewportWidth;
    if( sy < 0.0f ) sy += (FLOAT32)m_nViewportHeight - (FLOAT32)m_nGlyphHeight;

    FLOAT32 fOriginX = sx;
    FLOAT32 fOriginY = sy;

    FRMCOLOR nWhiteColor = 0x00ffffff | ( 0xff000000 & nColor.v );

    if( nFlags & FRM_FONT_RIGHT )
        sx -= GetTextWidth( strText );
    if( nFlags & FRM_FONT_CENTER )
        sx -= GetTextWidth( strText ) / 2;

#ifdef CORE_GL_CONTEXT
	GLuint vao;
	glGenVertexArrays(1, &vao);

	GLuint vbo;
	glGenBuffers(1, &vbo);
#else
    // Set the geoemtry
    glEnableVertexAttribArray( m_locVertexPos );
    glEnableVertexAttribArray( m_locVertexColor );
    glEnableVertexAttribArray( m_locVertexTex );
#endif

    while( *strText )
    {
        BYTE ch = *((BYTE*)strText);
        strText++;

        if( ch == '\n' )
        {
            sx  = fOriginX;
            sy += m_fScaleX * m_nGlyphHeight;

            if( nFlags & FRM_FONT_RIGHT )
                sx -= (FLOAT32)GetTextWidth( strText );

            if( nFlags & FRM_FONT_CENTER )
                sx -= (FLOAT32)GetTextWidth( strText ) / 2;
            
            continue;
        }

        FRM_FONT_GLYPH_GLES* pGlyph = &m_pGlyphsGLES[ch];
        sx += m_fScaleX * pGlyph->nOffset;

        FLOAT32 w = m_fScaleX * ( pGlyph->u1 - pGlyph->u0 );
        FLOAT32 h = m_fScaleY * ( pGlyph->v1 - pGlyph->v0 );

        struct FRM_FONT_VERTEX
        {
            FLOAT32 sx, sy;
            UINT32  nColor;
            INT16   tu, tv;
        };

        UINT32 nGlyphColor = ( ch >= 128 && ch < 160 ) ? nWhiteColor.v : nColor.v;

        FRM_FONT_VERTEX vQuad[4] =
        {
            { sx + 0, sy + 0, nGlyphColor, (INT16)pGlyph->u0, (INT16)pGlyph->v0 }, 
            { sx + 0, sy + h, nGlyphColor, (INT16)pGlyph->u0, (INT16)pGlyph->v1 }, 
            { sx + w, sy + h, nGlyphColor, (INT16)pGlyph->u1, (INT16)pGlyph->v1 }, 
            { sx + w, sy + 0, nGlyphColor, (INT16)pGlyph->u1, (INT16)pGlyph->v0 },
        };

#ifdef CORE_GL_CONTEXT
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(FRM_FONT_VERTEX) * 4, vQuad, GL_STATIC_DRAW);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glEnableVertexAttribArray( m_locVertexPos );		
		glVertexAttribPointer( m_locVertexPos,   2, GL_FLOAT,         FALSE, sizeof(FRM_FONT_VERTEX), 0 );
		glEnableVertexAttribArray( m_locVertexColor );
        glVertexAttribPointer( m_locVertexColor, 4, GL_UNSIGNED_BYTE, TRUE,  sizeof(FRM_FONT_VERTEX), (void*)8 );
		glEnableVertexAttribArray( m_locVertexTex );
        glVertexAttribPointer( m_locVertexTex,   2, GL_SHORT,         FALSE, sizeof(FRM_FONT_VERTEX), (void*)12 );
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		glBindVertexArray(vao);
#else
		glVertexAttribPointer( m_locVertexPos,   2, GL_FLOAT,         FALSE, sizeof(FRM_FONT_VERTEX), &vQuad[0].sx );
        glVertexAttribPointer( m_locVertexColor, 4, GL_UNSIGNED_BYTE, TRUE,  sizeof(FRM_FONT_VERTEX), &vQuad[0].nColor );
        glVertexAttribPointer( m_locVertexTex,   2, GL_SHORT,         FALSE, sizeof(FRM_FONT_VERTEX), &vQuad[0].tu );

#endif
        glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

        sx += m_fScaleX * pGlyph->nAdvance;
    }

#ifdef CORE_GL_CONTEXT
	glBindVertexArray(vao);
#endif
	glDisableVertexAttribArray( m_locVertexPos );
	glDisableVertexAttribArray( m_locVertexColor );
	glDisableVertexAttribArray( m_locVertexTex );
#ifdef CORE_GL_CONTEXT
	glBindVertexArray(0);
#endif

    m_fCursorX = sx;
    m_fCursorY = sy;

#if CORE_GL_CONTEXT
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
#endif 
    End();
}


//--------------------------------------------------------------------------------------
// Name: End()
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmFontGLES::End()
{
    if( --m_nNestedBeginCount == 0 )
    {
        // Restore render states
        glDisable( GL_BLEND );
        glEnable( GL_DEPTH_TEST );
    }
}

