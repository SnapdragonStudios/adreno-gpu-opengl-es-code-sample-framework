//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _FRM_RESOURCE_GLES_H_
#define _FRM_RESOURCE_GLES_H_

//#include "Crossplatform.h"

#if LINUX_OR_OSX && !defined(ANDROID)
#include <netinet/in.h>
#endif

#include "FrmStdLib.h"
#include "FrmPlatform.h"
#include "FrmResource.h"

#define GL_GLEXT_PROTOTYPES

#ifdef _OGLES3
#include "OpenGLES/FrmGLES3.h"
//#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>
#else
#ifdef CORE_GL_CONTEXT
#include <GLES3/gl3.h>
#endif
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif



//--------------------------------------------------------------------------------------
// Name: class CFrmTexture
// Desc: Resource wrapper for 2D textures
//--------------------------------------------------------------------------------------
class CFrmTexture : public CFrmResource
{
public:
    CFrmTexture() : CFrmResource( GL_TEXTURE_2D )
    {
        m_hTextureHandle = 0;
    }

    ~CFrmTexture()
    { 
        if( m_hTextureHandle )
            glDeleteTextures( 1, &m_hTextureHandle );
        m_hTextureHandle = 0;
    }

    VOID Bind( UINT32 nTextureUnit )
    {
        glActiveTexture( GL_TEXTURE0 + nTextureUnit );
        glBindTexture( GL_TEXTURE_2D, m_hTextureHandle );
        glEnable( GL_TEXTURE_2D );
    }

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nNumLevels;
    GLuint m_hTextureHandle;
};


//--------------------------------------------------------------------------------------
// Name: class CFrmTexture3D
// Desc: Resource wrapper for 3D textures
//--------------------------------------------------------------------------------------
class CFrmTexture3D : public CFrmResource
{
public:
    CFrmTexture3D() : CFrmResource( GL_TEXTURE_3D_OES )
    {
        m_hTextureHandle = 0;
    }

    ~CFrmTexture3D()
    { 
        if( m_hTextureHandle )
            glDeleteTextures( 1, &m_hTextureHandle );
        m_hTextureHandle = 0;
    }

    VOID Bind( UINT32 nTextureUnit )
    {
        glActiveTexture( GL_TEXTURE0 + nTextureUnit );
        glBindTexture( GL_TEXTURE_3D_OES, m_hTextureHandle );
        glEnable( GL_TEXTURE_3D_OES );
    }

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nDepth;
    GLuint m_hTextureHandle;
};


//--------------------------------------------------------------------------------------
// Name: class CFrmCubeMap
// Desc: Resource wrapper for cubemaps
//--------------------------------------------------------------------------------------
class CFrmCubeMap : public CFrmResource
{
public:
    CFrmCubeMap() : CFrmResource( GL_TEXTURE_CUBE_MAP )
    {
        m_hTextureHandle = 0;
    }

    ~CFrmCubeMap()
    { 
        if( m_hTextureHandle )
            glDeleteTextures( 1, &m_hTextureHandle );
        m_hTextureHandle = 0;
    }

    VOID Bind( UINT32 nTextureUnit )
    {
        glActiveTexture( GL_TEXTURE0 + nTextureUnit );
        glBindTexture( GL_TEXTURE_CUBE_MAP, m_hTextureHandle );
        glEnable( GL_TEXTURE_CUBE_MAP );
    }

    UINT32 m_nWidth;
    UINT32 m_nHeight;
    GLuint m_hTextureHandle;
};


BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nBaseSize, CFrmTexture** ppTexture );

BOOL FrmCreateTexture( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nBaseSize, GLint* pTextureHandle );

void FrmCreateTexture(
    GLuint* const textureHandlePtr,
    const GLenum textureSizedInternalFormat,
    const GLsizei widthPixels,
    const GLsizei heightPixels);

void FrmCreateAndBindFramebufferToTexture(
    GLuint* const frameBufferHandlePtr, 
    const GLuint colorTextureHandle, 
    const GLuint depthTextureHandle=GL_NONE);                       

BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nLevel,
                         UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nSize, CFrmTexture3D** ppTexture );

BOOL FrmCreateTexture3D( UINT32 nWidth, UINT32 nHeight, UINT32 nDepth, UINT32 nLevel,
                         UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                         VOID* pInitialData, UINT32 nSize, GLint* pTextureHandle );

BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nFaceSize, CFrmCubeMap** ppTexture );

BOOL FrmCreateCubeMap( UINT32 nWidth, UINT32 nHeight, UINT32 nNumLevels,
                       UINT32 nInternalPixelFormat, UINT32 nPixelType, UINT32 nBorder,
                       VOID* pInitialData, UINT32 nFaceSize, GLint* pTextureHandle );


//--------------------------------------------------------------------------------------
// Name: class CFrmVertexBuffer
// Desc: Resource wrapper for vertex buffer objects
//--------------------------------------------------------------------------------------
class CFrmVertexBuffer : public CFrmResource
{
public:
    CFrmVertexBuffer() : CFrmResource( GL_ARRAY_BUFFER )
    {
        m_hBufferHandle = 0;
    }

    ~CFrmVertexBuffer()
    { 
        glDeleteBuffers( 1, &m_hBufferHandle );
        m_hBufferHandle = 0;
    }

    VOID Bind( UINT32 nUnused ) 
    {
        glBindBuffer( GL_ARRAY_BUFFER, m_hBufferHandle );
    }

    UINT32 m_nNumVertices;
    UINT32 m_nVertexSize;
    GLuint m_hBufferHandle;
};

#ifdef CORE_GL_CONTEXT
UINT32 FrmCreateVertexArrayObject();
#endif

BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            CFrmVertexBuffer** ppVertexBuffer );
BOOL FrmCreateVertexBuffer( UINT32 nNumVertices, UINT32 nVertexSize, VOID* pSrcVertices,
                            GLuint* pVertexBufferHandle );


//--------------------------------------------------------------------------------------
// Name: class CFrmIndexBuffer
// Desc: Resource wrapper for index buffer (vertex array) objects
//--------------------------------------------------------------------------------------
class CFrmIndexBuffer : public CFrmResource
{
public:
    CFrmIndexBuffer() : CFrmResource( GL_ELEMENT_ARRAY_BUFFER )
    {
        m_hBufferHandle = 0;
    };
    
    ~CFrmIndexBuffer()
    { 
        glDeleteBuffers( 1, &m_hBufferHandle );
        m_hBufferHandle = 0;
    }

    VOID Bind( UINT32 nUnused ) 
    {
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_hBufferHandle );
    }

    UINT32 m_nNumIndices;
    UINT32 m_nIndexSize;
    GLuint m_hBufferHandle;
};


BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                           CFrmIndexBuffer** ppIndexBuffer );
BOOL FrmCreateIndexBuffer( UINT32 nNumIndices, UINT32 nIndexSize, VOID* pSrcIndices,
                           GLuint* pIndexBufferHandle );


//--------------------------------------------------------------------------------------
// Name: enum FRM_VERTEX_USAGE
// Desc: Pre-defined assignments for vertex attribute shader locations
//--------------------------------------------------------------------------------------
enum FRM_VERTEX_USAGE
{
    FRM_VERTEX_POSITION,          // 0
    FRM_VERTEX_BONEINDICES,       // 1
    FRM_VERTEX_BONEWEIGHTS,       // 2
    FRM_VERTEX_NORMAL,            // 3
    FRM_VERTEX_TANGENT,           // 4
    FRM_VERTEX_BINORMAL,          // 5
    FRM_VERTEX_TEXCOORD0,         // 6
    FRM_VERTEX_TEXCOORD1,         // 7
    FRM_VERTEX_TEXCOORD2,         // 8
    FRM_VERTEX_TEXCOORD3,         // 9
    FRM_VERTEX_TEXCOORD4,         // 10
    FRM_VERTEX_TEXCOORD5,         // 11
    FRM_VERTEX_COLOR0,            // 12
    FRM_VERTEX_COLOR1,            // 13
    FRM_VERTEX_COLOR2,            // 14
    FRM_VERTEX_COLOR3,            // 15
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_VERTEX_ELEMENT
// Desc: Description of a single vertex component
//--------------------------------------------------------------------------------------
#pragma pack(push,1)
struct FRM_VERTEX_ELEMENT
{
    UINT16 nUsage;        // FRM_VERTEX_USAGE
    UINT8  nSize;         // Number of components
    UINT16 nType;         // OpenGL data type
    UINT8  bNormalized;   // Whether to normalize the value
    UINT16 nStride;       // Stride of this component
//  BYTE* pStartAddress; //set to the address in the buffer of first index


#if LINUX_OR_OSX && !defined(ANDROID)
void ToHostEndian()
{
        nUsage= ntohs(nUsage);
        nType= ntohs(nType);
        nStride= ntohs(nStride);
}
#endif

};
#pragma pack(pop)


//--------------------------------------------------------------------------------------
// Name: class CFrmFrameBufferObject
// Desc: Wrapper class for rendering to offscreen textures via FBOs
//--------------------------------------------------------------------------------------
class CFrmFrameBufferObject
{
public:
    CFrmFrameBufferObject();
    ~CFrmFrameBufferObject();

    BOOL   Create( UINT32 nWidth, UINT32 nHeight, UINT32 nFormat, UINT32 nType );
    VOID   Destroy();
    VOID   Begin();
    VOID   End();
    UINT32 GetTexture() { return m_hTexture; }

public: 
    UINT32 m_nWidth;
    UINT32 m_nHeight;
    UINT32 m_nFormat;
    UINT32 m_nType;

    UINT32 m_hTexture;
    UINT32 m_hFrameBuffer;
    UINT32 m_hRenderBuffer;

    struct
    { 
        INT32 x, y, w, h;
    } m_PreviousViewport;
};

//--------------------------------------------------------------------------------------
// Name: FrmClearVertexAttributeArrays()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmClearVertexAttributeArrays()
{
    GLint i, iMaxAttribs = 0;
    glGetIntegerv( GL_MAX_VERTEX_ATTRIBS, &iMaxAttribs ); 
    for( i=0 ; i<iMaxAttribs ; i++ )
    {
        glDisableVertexAttribArray( (GLuint)i );
    }
}

//--------------------------------------------------------------------------------------
// Name: FrmSetTexture()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmSetTexture( UINT32 nTextureUnit, GLuint hTextureHandle )
{
    glActiveTexture( GL_TEXTURE0 + nTextureUnit );
    glBindTexture( GL_TEXTURE_2D, hTextureHandle );
}

#ifdef CORE_GL_CONTEXT
inline VOID FrmSetVertexArrayObect(GLuint hVertexArrayObject)
{
    glBindVertexArray(hVertexArrayObject);
}
#endif

//--------------------------------------------------------------------------------------
// Name: FrmSetVertexBuffer()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmSetVertexBuffer( GLuint hVertexBuffer )
{
    glBindBuffer( GL_ARRAY_BUFFER, hVertexBuffer );
}


//--------------------------------------------------------------------------------------
// Name: FrmSetIndexBuffer()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmSetIndexBuffer( GLuint hIndexBuffer )
{
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, hIndexBuffer );
}


//--------------------------------------------------------------------------------------
// Name: FrmSetVertexLayout()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmSetVertexLayout( FRM_VERTEX_ELEMENT* pVertexElements,
                                UINT32 nVertexSize, VOID* pBaseOffset = NULL )
{
    long nOffset = (long)pBaseOffset;

    while( pVertexElements->nSize )
    {
        glVertexAttribPointer( pVertexElements->nUsage, pVertexElements->nSize,
                               pVertexElements->nType, 
                               (GLboolean)pVertexElements->bNormalized,
                               nVertexSize, (VOID*)nOffset );
        glEnableVertexAttribArray( pVertexElements->nUsage );

        nOffset+= pVertexElements->nStride;
        pVertexElements++;
    }
}


//--------------------------------------------------------------------------------------
// Name: FrmDrawIndexedVertices()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmDrawIndexedVertices( UINT32 nPrimType, UINT32 nNumIndices,
                                    UINT32 nIndexSize, UINT32 nIndexOffset )
{
    UINT32 nType   = nIndexSize == sizeof(UINT32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
    VOID*  pOffset = reinterpret_cast<VOID*>( nIndexSize * nIndexOffset );

    glDrawElements( nPrimType, nNumIndices, nType, pOffset );
}

#if (_OGLES3)
//--------------------------------------------------------------------------------------
// Name: FrmDrawBuffers()
// Desc: 
//--------------------------------------------------------------------------------------
inline VOID FrmDrawBuffers( UINT32 n, const GLenum* buffers)
{
    glDrawBuffers(n, buffers);;
}
#endif

template<size_t kSize>
class ResourceHandleArray
{
public:
    void Initialize()
    {
        for (size_t i = 0; i < kSize; ++i)
        {
            m_resources[i] = GL_NONE;
        }
    }

    const GLuint& operator[](const size_t index) const
    {
        ADRENO_ASSERT(index < kSize, __FILE__, __LINE__);
        return m_resources[index];
    }
    GLuint& operator[](const size_t index)
    {
        return const_cast<GLuint&>(static_cast<const ResourceHandleArray*>(this)->operator[](index));
    }
    bool IsNull(const GLuint v)
    {
        return v == GL_NONE;
    }
    size_t Size() const
    {
        return kSize;
    }

private:
    GLuint m_resources[kSize];
};

#endif // _FRM_RESOURCE_GLES_H_
