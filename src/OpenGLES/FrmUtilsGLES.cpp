//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#include "OpenGLES/FrmUtilsGLES.h"

#include <GLES3/gl31.h>
#include <string.h>
#include "FrmFile.h"
#include "FrmPlatform.h"
#include "OpenGLES/FrmShader.h"


//--------------------------------------------------------------------------------------
// Name: FrmRenderTextureToScreen()
// Desc: Helper function to render a texture in screenspace -- FrmBlitAvoidGMemLoads() uses a triangle instead of a quad, and may provide superior performance
//--------------------------------------------------------------------------------------
VOID FrmRenderTextureToScreen_GLES( FLOAT32 sx, FLOAT32 sy, FLOAT32 fScale, 
                                    CFrmTexture* pTexture,
                                    INT32 hShaderProgram, INT32 hScreenSizeLoc )
{
    FrmRenderTextureToScreen_GLES( sx, sy, 
                                   fScale * pTexture->m_nWidth, 
                                   fScale * pTexture->m_nHeight, 
                                   pTexture->m_hTextureHandle, 
                                   hShaderProgram, hScreenSizeLoc );
}


VOID FrmRenderTextureToScreen_GLES( FLOAT32 sx, FLOAT32 sy, FLOAT32 w, FLOAT32 h,
                                    UINT32 hTexture,
                                    INT32 hShaderProgram, INT32 hScreenSizeLoc )
{
    struct { GLint x, y, width, height; } viewport;
    glGetIntegerv( GL_VIEWPORT, (GLint*)&viewport );

    FLOAT32 fTextureWidth  = w;
    FLOAT32 fTextureHeight = h;
    FLOAT32 fScreenWidth   = (FLOAT32)viewport.width;
    FLOAT32 fScreenHeight  = (FLOAT32)viewport.height;

    if( sx < 0.0f ) sx += fScreenWidth  - fTextureWidth;
    if( sy < 0.0f ) sy += fScreenHeight - fTextureHeight;

    sx = FrmFloor( sx );
    sy = FrmFloor( sy );

    // Set the geoemtry
    FRMVECTOR4 vQuad[] = 
    { 
        //          Screenspace x     Screenspace y      tu    tv
        FRMVECTOR4( sx,               sy,                0.0f, 1.0f ),
        FRMVECTOR4( sx,               sy+fTextureHeight, 0.0f, 0.0f ),
        FRMVECTOR4( sx+fTextureWidth, sy+fTextureHeight, 1.0f, 0.0f ),
        FRMVECTOR4( sx+fTextureWidth, sy,                1.0f, 1.0f ),
    };
#if CORE_GL_CONTEXT
 	GLuint vao;
 	glGenVertexArrays(1, &vao);
 	glBindVertexArray(vao);
 
 	GLuint vbo;
 	glGenBuffers(1, &vbo);
 	glBindBuffer(GL_ARRAY_BUFFER, vbo);
 	glBufferData(GL_ARRAY_BUFFER, sizeof(FRMVECTOR4) * 4, vQuad, GL_DYNAMIC_DRAW);
    glVertexAttribPointer( 0, 4, GL_FLOAT, 0, 0, 0 );
#else
	glVertexAttribPointer( 0, 4, GL_FLOAT, 0, 0, vQuad );
#endif
    glEnableVertexAttribArray( 0 );

    // Set the texture
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, hTexture );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glEnable( GL_BLEND );
    glDisable( GL_DEPTH_TEST );

    // Set the shader program
    glUseProgram( hShaderProgram );
    glUniform2f( hScreenSizeLoc, fScreenWidth, fScreenHeight );

    // Draw the quad
    glDrawArrays( GL_TRIANGLE_FAN, 0, 4 );

    glEnable( GL_DEPTH_TEST );
    glDisable( GL_BLEND );

#if CORE_GL_CONTEXT
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &vbo);
#endif 
}

//--------------------------------------------------------------------------------------
// Name: FrmBlitAvoidGMemLoads()
/* Desc:    glBlitFramebuffer() can cause unnecessary GMemLoads because it operates on system memory, bypassing tile-memory.  
            
            For example, if you blit from an offscreen buffer to the backbuffer, that might be okay, until you draw an overlay-font on the 
            backbuffer, at which point GL reloads pixels from system memory for each tile (incurring unnecessary GMemLoads) because blit didn’t 
            interface with tile-memory.

            This function performs the copy in a shader instead, allowing GL to collects all draw calls, and perform the copy-operation followed by 
            (for example) the font-overlay-draw – all in tile-memory, with no unnecessary GMemLoads.
*/
//--------------------------------------------------------------------------------------
void FrmBlitAvoidGMemLoads(const GLuint sourceTexture, const GLuint shaderProgramFullScreenCopy)
{
    // Set the texture
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, sourceTexture );

    // Set the shader program
    glUseProgram( shaderProgramFullScreenCopy );

    // Draw the full-screen triangle 
    glDrawArrays( GL_TRIANGLES, 0, 3 );            
}

//--------------------------------------------------------------------------------------
// Name: FrmReadFramebufferCpu()
// Desc: Read a framebuffer's texture into a Cpu buffer
//--------------------------------------------------------------------------------------
void FrmReadFramebufferCpu(
    UINT8* const readPixelsBuffer,
    const EGLint widthPixels,
    const EGLint heightPixels,
    const GLint frameBufferBaseInternalFormat,
    const GLint framebufferTextureType,
    const GLuint framebufferHandle)
{
    ADRENO_ASSERT(readPixelsBuffer, __FILE__, __LINE__);
    ADRENO_ASSERT(widthPixels > 0, __FILE__, __LINE__);
    ADRENO_ASSERT(heightPixels > 0, __FILE__, __LINE__);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebufferHandle);

    GLint framebufferSampleBuffers;
    glGetFramebufferParameteriv(GL_READ_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, &framebufferSampleBuffers);

    LOGI(   "glReadPixels(0, 0, %i, %i, 0x%x, 0x%x, %p) to framebufferHandle=%i which has GL_FRAMEBUFFER_DEFAULT_SAMPLES=%i",
            widthPixels, heightPixels, frameBufferBaseInternalFormat, framebufferTextureType, &readPixelsBuffer, framebufferHandle, framebufferSampleBuffers);

    glReadPixels(0, 0, widthPixels, heightPixels, frameBufferBaseInternalFormat, framebufferTextureType, readPixelsBuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}

//--------------------------------------------------------------------------------------
// Name: FrmGLExtensionSupported()
// Desc: Query for supporting a gles extension
//--------------------------------------------------------------------------------------
bool FrmGLExtensionSupported(const char*const extensionName)
{
    ADRENO_ASSERT(CStringNotEmpty(extensionName), __FILE__, __LINE__);
    
    GLint extensionsNum = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &extensionsNum);
    //LOGI("extensionNum=%i", extensionsNum);
    for (GLint i = 0; i < extensionsNum; ++i)
    {
        //LOGI("%s", reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))));
        if(strncmp(extensionName, reinterpret_cast<const char*>(glGetStringi(GL_EXTENSIONS, static_cast<GLuint>(i))), 1 << 8) == 0)
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------
// Name: DrawMeshVectors()
// Desc: Visualizes the mesh vectors using the supplied ModelViewProj matrix
//--------------------------------------------------------------------------------------
VOID CFrmShowMeshVectors::DrawMeshVectors( FRMMATRIX4X4 matMVP, FLOAT32 fLength )
{
    glUseProgram( m_hShowNormalsShader );
    glUniformMatrix4fv( glGetUniformLocation( m_hShowNormalsShader, "g_matModelViewProj" ), 
                        1, FALSE, (FLOAT32*)&matMVP );
    glUniform1f( glGetUniformLocation( m_hShowNormalsShader, "g_fMaxExtrusion" ), fLength );

    if( m_vNormals )   DrawVectors( m_vNormals,   FRMVECTOR3( 1.0f, 0.0f, 0.0f ) );
    if( m_vTangents )  DrawVectors( m_vTangents,  FRMVECTOR3( 0.0f, 1.0f, 0.0f ) );
    if( m_vBinormals ) DrawVectors( m_vBinormals, FRMVECTOR3( 0.0f, 0.0f, 1.0f ) );
}


//--------------------------------------------------------------------------------------
// Name: DrawVectors()
// Desc: Internal helper function to render either normals or tangents or binormals
//--------------------------------------------------------------------------------------
VOID CFrmShowMeshVectors::DrawVectors( FRMVECTOR3* pVectors, FRMVECTOR3 vColor )
{
    glUniform3fv( glGetUniformLocation( m_hShowNormalsShader, "g_vLineColor" ),
                  1, (FLOAT32*)&vColor );

    glVertexAttribPointer( 0, 3, GL_FLOAT, FALSE, 0, m_vPositions );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 1, 3, GL_FLOAT, FALSE, 0, pVectors );
    glEnableVertexAttribArray( 1 );
    glVertexAttribPointer( 2, 1, GL_FLOAT, FALSE, 0, m_fExtrusions );
    glEnableVertexAttribArray( 2 );

    glDrawArrays( GL_LINES, 0, 2*m_nNumVertices );
}


//--------------------------------------------------------------------------------------
// Name: Destroy()
// Desc: Cleanup function
//--------------------------------------------------------------------------------------
VOID CFrmShowMeshVectors::Destroy()
{
    if( m_hShowNormalsShader ) glDeleteProgram( m_hShowNormalsShader );
    delete[] m_fExtrusions;
    delete[] m_vPositions;
    delete[] m_vNormals;
    delete[] m_vTangents;
    delete[] m_vBinormals;

    m_hShowNormalsShader = 0;
    m_nNumVertices = 0;
    m_fExtrusions  = NULL;
    m_vPositions   = NULL;
    m_vNormals     = NULL;
    m_vTangents    = NULL;
    m_vBinormals   = NULL;
}

