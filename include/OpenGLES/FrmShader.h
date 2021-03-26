// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#ifndef _FRM_SHADER_H_
#define _FRM_SHADER_H_

#include "FrmPlatform.h"
#define GL_GLEXT_PROTOTYPES

#ifdef _OGLES3
#include "OpenGLES/FrmGLES3.h"
//#include <GLES3/gl3.h>
//#include <GLES2/gl2ext.h>
#else
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif


// Shader attribute type
struct FRM_SHADER_ATTRIBUTE
{
    const CHAR* strName;
    UINT32      nLocation;
};


// Helper functions for loading and compiling shaders
BOOL FrmCompileShaderFromString( const CHAR* strShaderSource, GLuint hShaderHandle );
BOOL FrmLoadShaderObject( const CHAR* strShaderSource, GLuint hShaderHandle );
BOOL FrmLoadShaderObjectFromFile( const CHAR* strFileName, GLuint hShaderHandle );


// Compiles a GLSL vertex shader and fragment shader into a binary shader program
BOOL FrmCompileShaderProgram( const CHAR* strVertexShaderSource,
                              const CHAR* strFragmentShaderSource, 
                              GLuint* pShaderProgramHandle,
                              const FRM_SHADER_ATTRIBUTE* pAttributes = NULL,
                              UINT32 nNumAttributes = 0 );


// Compiles a GLSL vertex shader and fragment shader into a binary shader program
BOOL FrmCompileShaderProgramFromFile( const CHAR* strVertexShaderFileName,
                                      const CHAR* strFragmentShaderFileName, 
                                      GLuint* pShaderProgramHandle,
                                      const FRM_SHADER_ATTRIBUTE* pAttributes = NULL,
                                      UINT32 nNumAttributes = 0 );

VOID FrmBindShaderAttributes( GLuint hShaderProgram,
                              const FRM_SHADER_ATTRIBUTE* pAttributes,
                              UINT32 nNumAttributes );

BOOL FrmLinkShaderProgram( UINT32 hShaderProgram );


// Loads a precompiled binary shader program
/*
BOOL FrmLoadShaderProgramFromFile( const CHAR* strBinaryShaderFileName,
                                   GLuint* pShaderProgramHandle,
                                   const FRM_SHADER_ATTRIBUTE* pAttributes = NULL,
                                   UINT32 nNumAttributes = 0 );
*/

#endif // _FRM_SHADER_H_
