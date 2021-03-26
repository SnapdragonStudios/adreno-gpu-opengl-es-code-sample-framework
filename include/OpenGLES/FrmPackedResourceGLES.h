// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_PACKED_RESOURCE_GLES_H_
#define _FRM_PACKED_RESOURCE_GLES_H_

#include "FrmPackedResource.h"
#include "OpenGLES/FrmResourceGLES.h"


//--------------------------------------------------------------------------------------
// Name: class CFrmPackedResourceGLES
// Desc: Loads and extracts GLES resources from a packed resource file
//--------------------------------------------------------------------------------------
class CFrmPackedResourceGLES : public CFrmPackedResource
{
public: 
    CFrmTexture*    GetTexture( const CHAR* strResource );
    INT32           GetTextureHandle( const CHAR* strResource );
    CFrmTexture3D*  GetTexture3D( const CHAR* strResource );
    INT32           GetTexture3DHandle( const CHAR* strResource );
    CFrmCubeMap*    GetCubeMap( const CHAR* strResource );
    INT32           GetCubeMapHandle( const CHAR* strResource );
};


#endif // _FRM_PACKED_RESOURCE_GLES_H_
