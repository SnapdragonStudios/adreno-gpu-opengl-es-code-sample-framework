//============================================================================================================
//
//
//                  Copyright (c) 2024, Qualcomm Innovation Center, Inc. All rights reserved.
//                              SPDX-License-Identifier: BSD-3-Clause
//
//============================================================================================================

#ifndef _FRM_MESH_H_
#define _FRM_MESH_H_

#include "FrmPlatform.h"
#include "OpenGLES/FrmResourceGLES.h"
#include "OpenGLES/FrmPackedResourceGLES.h"
#include "FrmResource.h"
#include "FrmMath.h"
//#include "Crossplatform.h"

#if LINUX_OR_OSX && !defined(ANDROID)
#include <algorithm>    // std::swap
#include <netinet/in.h>
#endif


//--------------------------------------------------------------------------------------
// Forward references
//--------------------------------------------------------------------------------------
struct FRM_MESH_FRAME;
struct FRM_ANIMATION;


//--------------------------------------------------------------------------------------
// Enums and defintions
//--------------------------------------------------------------------------------------
const UINT32 FRM_ANIMATION_TICKS_PER_SEC = 4800;


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH_TEXTURE
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_MESH_TEXTURE
{
    CHAR               m_strTexture[32];
    CFrmTexture*       m_pTexture;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH_BONE
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_MESH_BONE
{
    FRM_MESH_FRAME*   m_pFrame;             // The mesh frame associated with this bone
    FRMMATRIX4X4      m_matBoneOffset;


    #if LINUX_OR_OSX  && !defined(ANDROID)
    void ToHostEndian()
    {
        m_pFrame= (FRM_MESH_FRAME*)ntohl((unsigned long)m_pFrame);
        m_matBoneOffset.ToHostEndian();
    }
    #endif
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH_SUBSET
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_MESH_SUBSET
{
    UINT32             m_nPrimType;
    UINT32             m_nNumIndices;
    UINT32             m_nIndexOffset;

    UINT32*            m_pBoneIds;

    FRMVECTOR4         m_vDiffuseColor;
    FRMVECTOR4         m_vSpecularColor;
    FRMVECTOR3         m_vAmbientColor;
    FRMVECTOR3         m_vEmissiveColor;

    UINT32             m_nNumTextures;
    FRM_MESH_TEXTURE*  m_pTextures;

#if LINUX_OR_OSX && !defined(ANDROID)
    void ToHostEndian()
    {
      m_nPrimType= ntohl(m_nPrimType);
      m_nNumIndices= ntohl(m_nNumIndices);
      m_nIndexOffset= ntohl(m_nIndexOffset);
      m_pBoneIds= (UINT32*)ntohl((unsigned long)m_pBoneIds);
      m_nNumTextures= ntohl(m_nNumTextures);
      m_pTextures= (FRM_MESH_TEXTURE*)ntohl((unsigned long)m_pTextures);

      m_vDiffuseColor.ToHostEndian();
      m_vSpecularColor.ToHostEndian();
      m_vAmbientColor.ToHostEndian();
      m_vEmissiveColor.ToHostEndian();
    }
#endif
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_MESH
{
    FRM_VERTEX_ELEMENT m_VertexLayout[8];
    UINT32             m_Reserved;

    UINT32             m_nNumVertices;
    UINT16             m_nVertexSize;
    UINT32             m_hVertexBuffer;

    UINT32             m_nNumIndices;
    UINT16             m_nIndexSize;
    UINT32             m_hIndexBuffer;

    UINT32             m_nNumBonesPerVertex;
    UINT32             m_nNumBoneMatrices;
    UINT32             m_nNumBones;
    FRM_MESH_BONE*     m_pBones;

    UINT32             m_nNumSubsets;
    FRM_MESH_SUBSET*   m_pSubsets;


#if LINUX_OR_OSX && !defined(ANDROID)
    void ToHostEndian()
    {
        FRM_VERTEX_ELEMENT* pVertexElements= m_VertexLayout;
        while( pVertexElements->nSize )
        {
            pVertexElements->ToHostEndian();
            ++pVertexElements;
        }


        m_Reserved= ntohl(m_Reserved);
        m_nNumVertices= ntohl(m_nNumVertices);
        m_hVertexBuffer= ntohl(m_hVertexBuffer);
        m_nNumIndices= ntohl(m_nNumIndices);
        m_hIndexBuffer= ntohl(m_hIndexBuffer);
        m_nNumBonesPerVertex= ntohl(m_nNumBonesPerVertex);
        m_nNumBoneMatrices= ntohl(m_nNumBoneMatrices);
        m_nNumBonesPerVertex= ntohl(m_nNumBonesPerVertex);
        m_nNumBones= ntohl(m_nNumBones);
        m_pBones= (FRM_MESH_BONE*)ntohl((unsigned long)m_pBones);
        m_pSubsets= (FRM_MESH_SUBSET*)ntohl((unsigned long)m_pSubsets);

        m_nVertexSize= ntohs(m_nVertexSize);
        m_nIndexSize= ntohs(m_nIndexSize);
    }
#endif
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH_FRAME
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_ANIMATION;

const unsigned int MaxNameLength= 32;

struct FRM_MESH_FRAME
{
    CHAR               m_strName[32];

    FRMMATRIX4X4       m_matTransform;
    FRMMATRIX4X4       m_matBoneOffset2;
    FRMMATRIX4X4       m_matCombined;

    FRM_MESH*          m_pMesh;
    FRM_ANIMATION*     m_pAnimationData;

    FRM_MESH_FRAME*    m_pChild;
    FRM_MESH_FRAME*    m_pNext;


#if LINUX_OR_OSX && !defined(ANDROID)

    void ToHostEndian()
    {
        // Reverse "m_strName"..
        const uint Limit= (MaxNameLength/2);
        for(uint front=0;front<Limit;++front)
        {
            std::swap(m_strName[front],m_strName[MaxNameLength-1-front]);
        }

        m_matTransform.ToHostEndian();
        m_matBoneOffset2.ToHostEndian();
        m_matCombined.ToHostEndian();

        m_pMesh= (FRM_MESH*)ntohl((unsigned long)m_pMesh);
        m_pAnimationData= (FRM_ANIMATION*)ntohl((unsigned long)m_pAnimationData);
        m_pChild= (FRM_MESH_FRAME*)ntohl((unsigned long)m_pChild);
        m_pNext= (FRM_MESH_FRAME*)ntohl((unsigned long)m_pNext);
    }
#endif

};


//--------------------------------------------------------------------------------------
// Name: struct FRM_MESH_FILE_HEADER
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_MESH_FILE_HEADER
{
    UINT32 nMagic;
    UINT32 nSystemDataSize;
    UINT32 nBufferDataSize;
    UINT32 nNumFrames;
#if LINUX_OR_OSX && !defined(ANDROID)
    void ToHostEndian()
    {
      nMagic= ntohl(nMagic);
      nSystemDataSize= ntohl(nSystemDataSize);
      nBufferDataSize= ntohl(nBufferDataSize);
      nNumFrames= ntohl(nNumFrames);
    }
#endif

};

const UINT32 FRM_MESH_MAGIC_ID = ( ('M'<<0) | ('S'<<8) | ('H'<<16) | (1<<24) );


//--------------------------------------------------------------------------------------
// Name: struct FRM_ANIMATION_KEY
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_ANIMATION_KEY
{
    FRMVECTOR3         m_vScale;
    FRMVECTOR4         m_vRotate;
    FRMVECTOR3         m_vTranslate;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_ANIMATION
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_ANIMATION
{
    CHAR               m_strFrameName[32];
    BYTE               m_Reserved0[32];

    FRM_MESH_FRAME*    m_pMeshFrame;
    BYTE               m_Reserved1[40];

    UINT32             m_nNumKeys;
    UINT32*            m_pKeyTimes;
    FRM_ANIMATION_KEY* m_pKeyValues;
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_ANIMATION_SET
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_ANIMATION_SET
{
    CHAR               m_strName[64];
    UINT32             m_nPeriodInTicks;
    UINT32             m_nNumAnimations;
    FRM_ANIMATION*     m_pAnimations;    
};


//--------------------------------------------------------------------------------------
// Name: struct FRM_ANIMATION_FILE_HEADER
// Desc: 
//--------------------------------------------------------------------------------------
struct FRM_ANIMATION_FILE_HEADER
{
    UINT32 m_nMagicID;
    UINT32 m_nDataSize;
    UINT32 m_nAnimationsOffset;
    UINT32 m_nKeyTimesOffset;
    UINT32 m_nKeyValuesOffset;
};

const UINT32 FRM_ANIMATION_FILE_MAGIC_ID = (('A'<<0)|('N'<<8)|('I'<<16)|('M'<<24));


//--------------------------------------------------------------------------------------
// Name: FrmReadAnimation()
// Desc: Reads animation data from a file
//--------------------------------------------------------------------------------------
BOOL FrmReadAnimation( const CHAR* strFileName, FRM_ANIMATION_SET** ppAnimationSet );


//--------------------------------------------------------------------------------------
// Name: class CFrmMesh
// Desc: 
//--------------------------------------------------------------------------------------
class CFrmMesh
{
public:
    CFrmMesh()
    {
        m_pBufferData = NULL;
        m_pSystemData = NULL;
        m_pFrames     = NULL;
        m_nNumFrames  = 0;
    }

    ~CFrmMesh()
    {
        Destroy();
    }

    BOOL Load( const CHAR* strFileName );
    BOOL MakeDrawable( CFrmPackedResourceGLES* pResource = NULL );
    VOID Destroy();

    UINT32 GetNumSubsets( UINT32 nFrame )
    {
        if( NULL == m_pFrames[nFrame].m_pMesh )
            return 0;
        return m_pFrames[nFrame].m_pMesh->m_nNumSubsets;
    }

    FRM_MESH* GetMesh( UINT32 nFrame )
    {
        return m_pFrames[nFrame].m_pMesh;
    };

    FRM_MESH* GetMeshData( UINT32 nFrame, VOID** ppVertexData = NULL, 
                           VOID** ppIndexData = NULL )
    {
        FRM_MESH* pMesh = m_pFrames[nFrame].m_pMesh;
        if( NULL == pMesh )
            return NULL;

        if( m_pBufferData )
        {
            if( ppVertexData ) (*ppVertexData) = m_pBufferData + (UINT32)pMesh->m_hVertexBuffer;
            if( ppIndexData )  (*ppIndexData)  = m_pBufferData + (UINT32)pMesh->m_hIndexBuffer;
        }
        else
        {
            if( ppVertexData ) (*ppVertexData) = 0;
            if( ppIndexData )  (*ppIndexData)  = 0;
        }

        return pMesh;
    };

    VOID RenderSubset( UINT32 nFrame, UINT32 nSubset )
    {
        FRM_MESH* pMesh = m_pFrames[nFrame].m_pMesh;
        if( NULL == pMesh )
            return;
        FRM_MESH_SUBSET* pSubset = &pMesh->m_pSubsets[nSubset];



        for( UINT32 i=0; i<pSubset->m_nNumTextures; i++ )
        {
            FrmSetTexture( i, pSubset->m_pTextures[i].m_pTexture ? pSubset->m_pTextures[i].m_pTexture->m_hTextureHandle : static_cast<GLuint>(NULL) );
        }

#ifdef CORE_GL_CONTEXT
		FrmSetVertexArrayObect(m_hVertexArrayObject);
#endif
        FrmSetVertexBuffer( pMesh->m_hVertexBuffer );
		
		FrmClearVertexAttributeArrays();

        FrmSetVertexLayout( pMesh->m_VertexLayout, pMesh->m_nVertexSize );
        FrmSetIndexBuffer( pMesh->m_hIndexBuffer );
        FrmDrawIndexedVertices( pSubset->m_nPrimType, pSubset->m_nNumIndices, 
                                pMesh->m_nIndexSize, pSubset->m_nIndexOffset );
        FrmSetVertexBuffer( static_cast<GLuint>(NULL) );
#ifdef CORE_GL_CONTEXT
		FrmSetVertexArrayObect( NULL );
#endif
    }

    VOID Render()
    {
        for( UINT32 i = 0; i < m_nNumFrames; i++ )
        {
            FRM_MESH* pMesh = m_pFrames[i].m_pMesh;
            if( NULL == pMesh )
                continue;
        
            for( UINT32 j = 0; j < pMesh->m_nNumSubsets; j++ )
            {
                RenderSubset( i, j );
            }
        }
    }

public:
    UINT32          m_nSystemDataSize;
    UINT32          m_nBufferDataSize;
    BYTE*           m_pSystemData;
    BYTE*           m_pBufferData;

    FRM_MESH_FRAME* m_pFrames;
    UINT32          m_nNumFrames;

#ifdef CORE_GL_CONTEXT
	// core 3.2 req
	UINT32			   m_hVertexArrayObject;
#endif
};


struct FRM_ANIMATION_BLENDER
{
    FRM_ANIMATION_SET* m_pAnimationSet;

    UINT32             m_nNumAnimations;
    FRM_ANIMATION_KEY* m_pAnimationKeys;
    FRM_MESH_FRAME**   m_ppMeshFrames;

    VOID SetAnimation( FRM_ANIMATION_SET* pAnimationSet, UINT32 nPeriodicAnimTime );    

    FRM_ANIMATION_BLENDER()
    {
        m_pAnimationSet  = NULL;
        m_nNumAnimations = 0;
        m_pAnimationKeys = NULL;
        m_ppMeshFrames   = NULL;
    }

    ~FRM_ANIMATION_BLENDER()
    {
        Destroy();
    }

    VOID Destroy()
    {
        delete[] m_pAnimationKeys;
        delete[] m_ppMeshFrames;

        m_pAnimationSet  = NULL;
        m_nNumAnimations = 0;
        m_pAnimationKeys = NULL;
        m_ppMeshFrames   = NULL;
    }
};


//--------------------------------------------------------------------------------------
// Name: FrmReadAnimation()
// Desc: Reads animation data from a file
//--------------------------------------------------------------------------------------
BOOL FrmReadAnimation( const CHAR* strFileName, FRM_ANIMATION_SET** ppAnimationSet );


//--------------------------------------------------------------------------------------
// Name: FrmWriteAnimation()
// Desc: Reads animation data from a file
//--------------------------------------------------------------------------------------
BOOL FrmWriteAnimation( const CHAR* strFileName, FRM_ANIMATION_SET* pAnimationSet );


//--------------------------------------------------------------------------------------
// Name: FrmCreateAnimationBlender()
// Desc: Creates an animation blender
//--------------------------------------------------------------------------------------
VOID FrmCreateAnimationBlender( UINT32 nNumAnimations,
                                FRM_ANIMATION_SET* pAnimationSet, 
                                FRM_MESH_FRAME* pFrames, UINT32 nNumFrames,
                                FRM_ANIMATION_BLENDER** ppAnimBlender );

#endif // _FRM_MESH_H_
