// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmMesh.h"
#include "FrmFile.h"


#include "FrmUtils.h"


#include "FrmFont.h"
//#include "Crossplatform.h"


#if LINUX_OR_OSX && !defined(ANDROID)
#include <netinet/in.h>
#endif
#include <stdio.h>

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmMesh::Load( const CHAR* strFileName )
{
    // Open the file
#ifndef UNDER_CE
    FRM_FILE* file;
#else
    FILE* file;
#endif // UNDER_CE
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &file ) )
    {
        return FALSE;
    }


#if LINUX_OR_OSX && !defined(ANDROID)
    bool isBigEndian= IsBigEndian();
#endif

    



    // Read in the mesh header
    FRM_MESH_FILE_HEADER header;
    FrmFile_Read( file, &header, sizeof(header) );

     
    
    
    
    char *cp1, *cp2, *cp3;


     cp3= (char *)&FRM_FONT_FILE_MAGIC_ID;

#if LINUX_OR_OSX && !defined(ANDROID)
     // If little endian...
    if(true==isBigEndian)
    {
        cp1= (char *)&FRM_MESH_MAGIC_ID;
        cp2= (char *)&header.nMagic;
        header.ToHostEndian();  // Convert to little endian..
    }
#endif

    // If the header number is not the same as we know it should be...
    if( header.nMagic != FRM_MESH_MAGIC_ID )
    {

        cp1= (char *)&FRM_MESH_MAGIC_ID;
        cp2= (char *)&header.nMagic;


        // Failure..
        FrmFile_Close( file );
        return FALSE;
    }




    m_nNumFrames      = header.nNumFrames;
    m_nSystemDataSize = header.nSystemDataSize;
    m_nBufferDataSize = header.nBufferDataSize;

    // Read in the data
    m_pSystemData = new BYTE[m_nSystemDataSize];
    m_pBufferData = new BYTE[m_nBufferDataSize];
    FrmFile_Read( file, m_pSystemData, m_nSystemDataSize );
    FrmFile_Read( file, m_pBufferData, m_nBufferDataSize );

    // Done with the file
    FrmFile_Close( file );

    // Fix up pointers
    m_pFrames = (FRM_MESH_FRAME*)m_pSystemData;

    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {   // TODO: Reverse pFrame bytes if need be..
        FRM_MESH_FRAME* pFrame = &m_pFrames[i];

#if LINUX_OR_OSX && !defined(ANDROID)
        // If little endian...
        if(true==isBigEndian)
        {
            pFrame->ToHostEndian();
        }
#endif

        // Fix up child and next pointers
        if( pFrame->m_pChild )
            pFrame->m_pChild = (FRM_MESH_FRAME*)( m_pSystemData + (unsigned long)pFrame->m_pChild );
        if( pFrame->m_pNext )
            pFrame->m_pNext = (FRM_MESH_FRAME*)( m_pSystemData + (unsigned long)pFrame->m_pNext );

        if( pFrame->m_pMesh )
        {
            // Fix up mesh pointer
            pFrame->m_pMesh = (FRM_MESH*)( m_pSystemData + (unsigned long)pFrame->m_pMesh );
            // TODO: Reverse pMesh bytes if need be..            
            FRM_MESH* pMesh = pFrame->m_pMesh;
#if LINUX_OR_OSX && !defined(ANDROID)
            // If little endian...
            if(true==isBigEndian)
            {
                pMesh->ToHostEndian();
            }
#endif

            // Fix up bones pointer
            if( pMesh->m_pBones )
            {
                pMesh->m_pBones = (FRM_MESH_BONE*)( m_pSystemData + (unsigned long)pMesh->m_pBones );
                FRM_MESH_BONE* pBones = pMesh->m_pBones;


                for( UINT32 j=0; j<pMesh->m_nNumBones; j++ )
                {
                    #if LINUX_OR_OSX && !defined(ANDROID)
                    // If little endian...
                    if(true==isBigEndian)
                    {
                        pBones[j].ToHostEndian();
                    }

                    #endif

                    pBones[j].m_pFrame = (FRM_MESH_FRAME*)( m_pSystemData + (unsigned long)pBones[j].m_pFrame );
                }
            }

            // Fix up subsets pointer
            if( pMesh->m_pSubsets )
            {
                pMesh->m_pSubsets = (FRM_MESH_SUBSET*)( m_pSystemData + (unsigned long)pMesh->m_pSubsets );
                FRM_MESH_SUBSET* pSubsets = pMesh->m_pSubsets;

                

                for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
                {
                    #if LINUX_OR_OSX && !defined(ANDROID)
                    // If little endian...
                    if(true==isBigEndian)
                    {
                        pSubsets[j].ToHostEndian();
                    }
                    #endif

					// Ensure the mesh prim type is set
					if( pSubsets[j].m_nPrimType > 6 )
						pSubsets[j].m_nPrimType = GL_TRIANGLES;

                    // Fix up texture pointers
                    if( pSubsets[j].m_pTextures )
                        pSubsets[j].m_pTextures = (FRM_MESH_TEXTURE*)( m_pSystemData + (unsigned long)pSubsets[j].m_pTextures );

                    // Fix up bone subset pointers
                    if( pSubsets[j].m_pBoneIds )
                        pSubsets[j].m_pBoneIds = (UINT32*)( m_pSystemData + (unsigned long)pSubsets[j].m_pBoneIds );
                }
            }
        }
    }

    return TRUE;
}
    

//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
BOOL CFrmMesh::MakeDrawable( CFrmPackedResourceGLES* pResource )
{
    // Create vertex and index buffers
    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {
        FRM_MESH* pMesh = m_pFrames[i].m_pMesh;
        if( pMesh )
        {
            // Create the vertex buffer
            BYTE* pVertexData = m_pBufferData + (UINT32)pMesh->m_hVertexBuffer;
#ifdef CORE_GL_CONTEXT
		    m_hVertexArrayObject = FrmCreateVertexArrayObject();
#endif
            if( FALSE == FrmCreateVertexBuffer( pMesh->m_nNumVertices, 
                                                pMesh->m_nVertexSize, 
                                                pVertexData, &pMesh->m_hVertexBuffer ) )
                return FALSE;

            // Create the index buffer
            BYTE* pIndexData  = m_pBufferData + (UINT32)pMesh->m_hIndexBuffer;
			
			// Force 2 byte indices
			if( pMesh->m_nIndexSize == sizeof(UINT32) )
			{
				UINT32* pSrcIndexData = (UINT32*)pIndexData;
				UINT16* pDstIndexData = (UINT16*)pIndexData;

				// Convert 4 byte indices to 2 bytes
				for( UINT32 i=0; i<pMesh->m_nNumIndices; i++ )
				{
					pDstIndexData[i] = (UINT16)pSrcIndexData[i];
				}

				pMesh->m_nIndexSize = sizeof(UINT16);
			}

            if( FALSE == FrmCreateIndexBuffer( pMesh->m_nNumIndices, 
                                               pMesh->m_nIndexSize, 
                                               pIndexData, &pMesh->m_hIndexBuffer ) )
                return FALSE;

            // Create the textures
            if( pResource )
            {
                for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
                {
                    FRM_MESH_SUBSET* pSubset = &pMesh->m_pSubsets[j];
                    for( UINT32 k=0; k<pSubset->m_nNumTextures; k++ )
                    {
                        const CHAR* strTexture = pSubset->m_pTextures[k].m_strTexture;
                        CFrmTexture* pTexture  = pResource->GetTexture( strTexture );
                        pSubset->m_pTextures[k].m_pTexture = pTexture;
                    }
                }
            }
        }
    }
        
    // Done with the buffer data, so okay to delete it now
    delete[] m_pBufferData;
    m_pBufferData = NULL;

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
VOID CFrmMesh::Destroy()
{
    for( UINT32 i=0; i<m_nNumFrames; i++ )
    {
        FRM_MESH* pMesh = m_pFrames[i].m_pMesh;
        if( pMesh )
        {
            // Release vertex and index buffers
            if( m_pBufferData == NULL )
            {
                glDeleteBuffers( 1, &pMesh->m_hVertexBuffer );
                glDeleteBuffers( 1, &pMesh->m_hIndexBuffer );
            }

            // Release any textures
            for( UINT32 j=0; j<pMesh->m_nNumSubsets; j++ )
            {
                FRM_MESH_SUBSET* pSubset = &pMesh->m_pSubsets[j];
                for( UINT32 k=0; k<pSubset->m_nNumTextures; k++ )
                {
                    if( pSubset->m_pTextures[k].m_pTexture )
                        pSubset->m_pTextures[k].m_pTexture->Release();
                }
            }
        }
    }

    if( m_pBufferData ) delete[] m_pBufferData;
    if( m_pSystemData ) delete[] m_pSystemData;

    m_pBufferData = NULL;
    m_pSystemData = NULL;
    m_pFrames     = NULL;
    m_nNumFrames  = 0;
}


//--------------------------------------------------------------------------------------
// Name: FrmReadAnimation
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmReadAnimation( const CHAR* strFileName, FRM_ANIMATION_SET** ppAnimationSet )
{
    // Open the file for reading
    // Open the file
    FRM_FILE* file;
    if( FALSE == FrmFile_Open( strFileName, FRM_FILE_READ, &file ) )
        return FALSE;

    // Read the file header
    FRM_ANIMATION_FILE_HEADER Header;
    FrmFile_Read( file, &Header, sizeof(Header) );
    if( Header.m_nMagicID != FRM_ANIMATION_FILE_MAGIC_ID )
    {
        FrmFile_Close( file );
        return FALSE;
    }

    // Allocate memory and read the file
    BYTE* pData = new BYTE[Header.m_nDataSize];
    FrmFile_Read( file, pData, Header.m_nDataSize );
    FrmFile_Close( file );

    // Patch up the pointers to the animation key tables
    FRM_ANIMATION_SET* pAnimationSet = (FRM_ANIMATION_SET*)pData;

    pAnimationSet->m_pAnimations = (FRM_ANIMATION*)&pData[(unsigned long)pAnimationSet->m_pAnimations];
    for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
    {
        FRM_ANIMATION* pAnimation = &pAnimationSet->m_pAnimations[i];
        pAnimation->m_pKeyTimes  = (UINT32*)&pData[(unsigned long)pAnimation->m_pKeyTimes];
        pAnimation->m_pKeyValues = (FRM_ANIMATION_KEY*)&pData[(unsigned long)pAnimation->m_pKeyValues];
    }

    (*ppAnimationSet) = pAnimationSet;
    return TRUE;
}

//--------------------------------------------------------------------------------------
// Name: FrmWriteAnimation()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmWriteAnimation( const CHAR* strFileName, FRM_ANIMATION_SET* pAnimationSet )
{
	// Open the file for writing
	FRM_FILE* file;
	if( FALSE == FrmFile_Open( strFileName, FRM_FILE_WRITE, &file ) )
		return FALSE;

	// Compute the size requirements
	UINT32 nAnimationsSize = 0;
	UINT32 nKeyTimesSize   = 0;
	UINT32 nKeyValuesSize  = 0;

	for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
	{
		nAnimationsSize += sizeof(FRM_ANIMATION);
		nKeyTimesSize   += sizeof(UINT32) * pAnimationSet->m_pAnimations[i].m_nNumKeys;
		nKeyValuesSize  += sizeof(FRM_ANIMATION_KEY) * pAnimationSet->m_pAnimations[i].m_nNumKeys;
	}

	UINT32 nAnimationsOffset = sizeof(FRM_ANIMATION_SET);
	UINT32 nKeyTimesOffset   = nAnimationsOffset + nAnimationsSize;
	UINT32 nKeyValuesOffset  = nKeyTimesOffset + nKeyTimesSize;

	// Write the file header
	FRM_ANIMATION_FILE_HEADER Header;
	Header.m_nMagicID  = FRM_ANIMATION_FILE_MAGIC_ID;
	Header.m_nDataSize = sizeof(FRM_ANIMATION_SET) + nAnimationsSize + nKeyTimesSize + nKeyValuesSize;
	Header.m_nAnimationsOffset = nAnimationsOffset;
	Header.m_nKeyTimesOffset   = nKeyTimesOffset;
	Header.m_nKeyValuesOffset  = nKeyValuesOffset;
	FrmFile_Write( file, &Header, sizeof(Header) );

	// Write the root animation set structure
	FRM_ANIMATION_SET DstAnimSet = {0};
	FrmStrcpy( DstAnimSet.m_strName, pAnimationSet->m_strName );
	DstAnimSet.m_nPeriodInTicks = pAnimationSet->m_nPeriodInTicks;
	DstAnimSet.m_nNumAnimations = pAnimationSet->m_nNumAnimations;
	DstAnimSet.m_pAnimations    = (FRM_ANIMATION*)nAnimationsOffset;
	FrmFile_Write( file, &DstAnimSet, sizeof(DstAnimSet) );

	// Write the animations
	for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
	{
		FRM_ANIMATION* pSrcAnim = &pAnimationSet->m_pAnimations[i];
		FRM_ANIMATION DstAnim = {0};
		FrmStrcpy( DstAnim.m_strFrameName, pSrcAnim->m_strFrameName );
		DstAnim.m_nNumKeys   = pSrcAnim->m_nNumKeys;
		DstAnim.m_pKeyTimes  = (UINT32*)nKeyTimesOffset;
		DstAnim.m_pKeyValues = (FRM_ANIMATION_KEY*)nKeyValuesOffset;
		FrmFile_Write( file, &DstAnim, sizeof(DstAnim) );

		nKeyTimesOffset  += DstAnim.m_nNumKeys * sizeof(UINT32);
		nKeyValuesOffset += DstAnim.m_nNumKeys * sizeof(FRM_ANIMATION_KEY);
	}

	// Write the key times
	for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
	{
		FRM_ANIMATION* pSrcAnim = &pAnimationSet->m_pAnimations[i];
		FrmFile_Write( file, pSrcAnim->m_pKeyTimes, sizeof(UINT32) * pSrcAnim->m_nNumKeys );
	}

	// Write the key values
	for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
	{
		FRM_ANIMATION* pSrcAnim = &pAnimationSet->m_pAnimations[i];
		FrmFile_Write( file, pSrcAnim->m_pKeyValues, sizeof(FRM_ANIMATION_KEY) * pSrcAnim->m_nNumKeys );
	}

	// Close the file
	FrmFile_Close( file );

	return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: 
// Desc: 
//--------------------------------------------------------------------------------------
VOID FrmCreateAnimationBlender( UINT32 nNumAnimations,
	FRM_ANIMATION_SET* pAnimationSet, 
	FRM_MESH_FRAME* pFrames, UINT32 nNumFrames,
	FRM_ANIMATION_BLENDER** ppAnimBlender )
{
	FRM_ANIMATION_BLENDER* pAnimBlender = new FRM_ANIMATION_BLENDER;
	pAnimBlender->m_nNumAnimations = nNumAnimations;
	pAnimBlender->m_ppMeshFrames   = new FRM_MESH_FRAME*[nNumAnimations];
	pAnimBlender->m_pAnimationKeys = new FRM_ANIMATION_KEY[nNumAnimations];
	(*ppAnimBlender) = pAnimBlender;

	for( UINT32 i=0; i<pAnimationSet->m_nNumAnimations; i++ )
	{
		FRM_ANIMATION* pAnimation = &pAnimationSet->m_pAnimations[i];
		for( UINT32 j=0; j<nNumFrames; j++ )
		{
			FRM_MESH_FRAME* pFrame = &pFrames[j];
			if( !FrmStricmp( pFrame->m_strName, pAnimation->m_strFrameName ) )
			{
				pAnimBlender->m_ppMeshFrames[i] = pFrame;
				break;
			}
		}
	}
}

// This function is a combination of FRM_ANIMATION_SET::SetTime() and 
// FRM_ANIMATION_BLENDER::ApplyAnimationsToFrames() from the old framework
// references to the old framework can be found in the Robot demo deleted files
// implementation is similar to SetAnimationTime in Scene.cpp for the Skinning demo
VOID FRM_ANIMATION_BLENDER::SetAnimation( FRM_ANIMATION_SET* pAnimationSet, UINT32 nPeriodicAnimTime )
{	
	m_pAnimationSet = pAnimationSet;

	// set time
	nPeriodicAnimTime = nPeriodicAnimTime % pAnimationSet->m_nPeriodInTicks;

	FRMVECTOR3         m_vCurrentScale;
	FRMVECTOR4         m_vCurrentRotate;
	FRMVECTOR3         m_vCurrentTranslate;

	for( UINT32 i=0; i<m_nNumAnimations; i++ )
	{
		FRM_ANIMATION* pAnimation = &pAnimationSet->m_pAnimations[i];

		UINT32 key1 = 0;
		while( pAnimation->m_pKeyTimes[key1] < nPeriodicAnimTime )
			key1++;
		if( key1 == 0 )
		{
			FRM_ANIMATION_KEY* pKey1 = &pAnimation->m_pKeyValues[key1];
			m_pAnimationKeys[i].m_vScale      = pKey1->m_vScale;
			m_pAnimationKeys[i].m_vRotate    = pKey1->m_vRotate;
			m_pAnimationKeys[i].m_vTranslate = pKey1->m_vTranslate;
		}
		else
		{
			UINT32 key0 = key1 ? key1 - 1 : 0;

			UINT32  nTime0 = pAnimation->m_pKeyTimes[key0];
			UINT32  nTime1 = pAnimation->m_pKeyTimes[key1];
			FLOAT32 fLerp = (FLOAT32)( nPeriodicAnimTime - nTime0 ) / (FLOAT32)( nTime1 - nTime0 );

			FRM_ANIMATION_KEY* pKey0 = &pAnimation->m_pKeyValues[key0];
			FRM_ANIMATION_KEY* pKey1 = &pAnimation->m_pKeyValues[key1];

			m_pAnimationKeys[i].m_vScale     = FrmVector3Lerp( pKey0->m_vScale, pKey1->m_vScale, fLerp );
			m_pAnimationKeys[i].m_vRotate    = FrmVector4SLerp( pKey0->m_vRotate, pKey1->m_vRotate, fLerp );
			m_pAnimationKeys[i].m_vTranslate = FrmVector3Lerp( pKey0->m_vTranslate, pKey1->m_vTranslate, fLerp );
		}

		// Tie the animation into the linked frame
		FRM_MESH_FRAME* pFrame = m_ppMeshFrames[i];
		if( NULL == pFrame )
			continue;

		FRMMATRIX4X4 matScale     = FrmMatrixScale( m_pAnimationKeys[i].m_vScale );
		FRMMATRIX4X4 matRotate    = FrmMatrixRotate( m_pAnimationKeys[i].m_vRotate );
		FRMMATRIX4X4 matTranslate = FrmMatrixTranslate( m_pAnimationKeys[i].m_vTranslate );

		pFrame->m_matTransform = FrmMatrixMultiply( matRotate, matScale );
		pFrame->m_matTransform = FrmMatrixMultiply( pFrame->m_matTransform, matTranslate );

	}
}



