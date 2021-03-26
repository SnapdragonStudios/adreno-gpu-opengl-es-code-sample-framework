// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmPlatform.h"
#include "FrmShader.h"
#include "FrmResource.h"
#include "FrmUtilsGLES.h"


//--------------------------------------------------------------------------------------
// Name: FrmBindShaderAttributes()
// Desc: Bind the attributes to the shader program
//--------------------------------------------------------------------------------------
VOID FrmBindShaderAttributes( GLuint hShaderProgram,
                              const FRM_SHADER_ATTRIBUTE* pAttributes,
                              UINT32 nNumAttributes )
{
    // If specific attributes were supplied, use though	
    if( nNumAttributes )
    {
        for( UINT32 i=0; i<nNumAttributes; i++ )
        {
            glBindAttribLocation( hShaderProgram, pAttributes[i].nLocation,
                                                  pAttributes[i].strName );
        }
    }
    else // Otherwise, bind some generic name mappings
    {
        glBindAttribLocation( hShaderProgram, 0, "FRM_VERTEX_ATTRIBUTE_0" );
        glBindAttribLocation( hShaderProgram, 1, "FRM_VERTEX_ATTRIBUTE_1" );
        glBindAttribLocation( hShaderProgram, 2, "FRM_VERTEX_ATTRIBUTE_2" );
        glBindAttribLocation( hShaderProgram, 3, "FRM_VERTEX_ATTRIBUTE_3" );
        glBindAttribLocation( hShaderProgram, 4, "FRM_VERTEX_ATTRIBUTE_4" );
        glBindAttribLocation( hShaderProgram, 5, "FRM_VERTEX_ATTRIBUTE_5" );
        glBindAttribLocation( hShaderProgram, 6, "FRM_VERTEX_ATTRIBUTE_6" );
        glBindAttribLocation( hShaderProgram, 7, "FRM_VERTEX_ATTRIBUTE_7" );

        glBindAttribLocation( hShaderProgram, FRM_VERTEX_POSITION,    "FRM_VERTEX_POSITION" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_BONEWEIGHTS, "FRM_VERTEX_BONEWEIGHTS" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_BONEINDICES, "FRM_VERTEX_BONEINDICES" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_NORMAL,      "FRM_VERTEX_NORMAL" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_TANGENT,     "FRM_VERTEX_TANGENT" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_BINORMAL,    "FRM_VERTEX_BINORMAL" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_TEXCOORD0,   "FRM_VERTEX_TEXCOORD0" );
        glBindAttribLocation( hShaderProgram, FRM_VERTEX_TEXCOORD1,   "FRM_VERTEX_TEXCOORD1" );
    }
}


//--------------------------------------------------------------------------------------
// Name: FrmCompileShaderFromString()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmCompileShaderFromString( const CHAR* strShaderSource, GLuint hShaderHandle )
{
    glShaderSource( hShaderHandle, 1, &strShaderSource, NULL );
    #if OSX
    // <SRAUT>: glShaderSource causes 1280 on OSX for only GLES30 samples
    // TODO: Investigate later as this doesn't affect the sample output
    glGetError();
    #endif
    glCompileShader( hShaderHandle );

    // Check for compile success
    GLint nCompileResult = 0;
    glGetShaderiv( hShaderHandle, GL_COMPILE_STATUS, &nCompileResult );
    if( 0 == nCompileResult )
    {
        CHAR strInfoLog[1024];
        GLint nLength;
        glGetShaderInfoLog( hShaderHandle, 1024, &nLength, strInfoLog );
        FrmLogMessage( strInfoLog );
        return FALSE;
    }
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmLoadShaderObjectFromFile()
// Desc: Install the given shaders into OpenGL for use by the pipeline.
//--------------------------------------------------------------------------------------
BOOL FrmLoadShaderObjectFromFile( const CHAR* strFileName, GLuint hShaderHandle )
{
    CHAR* strShaderSource;
    if( FALSE == FrmLoadFile( strFileName, (VOID**)&strShaderSource ) )
    {
        FrmLogMessage( "ERROR: Could not load shader file '", strFileName, "'\n" );
        return FALSE;
    }

    BOOL bResult = FrmCompileShaderFromString( strShaderSource, hShaderHandle );
    if( FALSE == bResult )
    {
        FrmLogMessage( "ERROR: Could not load shader file '", strFileName, "'\n" );
    }

    FrmUnloadFile( strShaderSource );
    return bResult;
}


//--------------------------------------------------------------------------------------
// Name: FrmLinkShaderProgram()
// Desc: Helper function to link a shader program
//--------------------------------------------------------------------------------------
BOOL FrmLinkShaderProgram( UINT32 hShaderProgram )
{
    // Link the whole program together
    glLinkProgram( hShaderProgram );

    // Check for link success
    GLint LinkStatus;
    glGetProgramiv( hShaderProgram, GL_LINK_STATUS, &LinkStatus );
    if( FALSE == LinkStatus )
    {
        CHAR  strInfoLog[1024];
        INT32 nLength;
        glGetProgramInfoLog( hShaderProgram, 1024, &nLength, strInfoLog );
        FrmLogMessage( strInfoLog, "\n" );
        return FALSE;
    }

    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCompileShaderProgram()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmCompileShaderProgram( const CHAR* strVertexShader,
                              const CHAR* strFragmentShader, 
                              GLuint* pShaderProgramHandle,
                              const FRM_SHADER_ATTRIBUTE* pAttributes,
                              UINT32 nNumAttributes )
{
    // Create the object handles
    GLuint hVertexShader   = glCreateShader( GL_VERTEX_SHADER );
    GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    // Compile the shaders
    if( !FrmCompileShaderFromString( strVertexShader, hVertexShader ) )
    {
        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        return FALSE;
    }
    if( !FrmCompileShaderFromString( strFragmentShader, hFragmentShader ) )
    {
        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        return FALSE;
    }

    // Attach the individual shaders to the common shader program
    GLuint hShaderProgram  = glCreateProgram();
    glAttachShader( hShaderProgram, hVertexShader );
    glAttachShader( hShaderProgram, hFragmentShader );

    // Bind the shader attributes
    FrmBindShaderAttributes( hShaderProgram, pAttributes, nNumAttributes );

    // Link the vertex shader and fragment shader together
    if( FALSE == FrmLinkShaderProgram( hShaderProgram ) )
    {
        glDeleteProgram( hShaderProgram );
        return FALSE;
    }

	// SRAUT - Deleting shaders here causes a glError, found this while calling FrmCompileShaderProgram for the font shaders
	// Investigate more in the next release - Jira ACGSDK 989
	// From comments for mglDeleteShader:
	// We can't delete the shader object here.  If it is currently bound to a program it can't be deleted until it's unbound
	// Tracking that is extra work.  It seems like I can just never delete these string and it will just waste a little extra
	// memory.  Every place we use this we check to see if this id is a shader object first.  I might want to add this tracking
	// at a future date, but I don't see the need right now.
	
    // It's now safe to toss away the original shader handles

	// Comment this out for now so we can run samples for releasing v3.6
	//glDeleteShader( hVertexShader );
    //glDeleteShader( hFragmentShader );

    // Return the shader program
    (*pShaderProgramHandle) = hShaderProgram;
    return TRUE;
}


//--------------------------------------------------------------------------------------
// Name: FrmCompileShaderProgramFromFile()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmCompileShaderProgramFromFile( const CHAR* strVertexShaderFileName,
                                      const CHAR* strFragmentShaderFileName, 
                                      GLuint* pShaderProgramHandle,
                                      const FRM_SHADER_ATTRIBUTE* pAttributes,
                                      UINT32 nNumAttributes )
{
    // Create the object handles
    GLuint hVertexShader   = glCreateShader( GL_VERTEX_SHADER );
    GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );

    // Compile the shaders
    if( !FrmLoadShaderObjectFromFile( strVertexShaderFileName, hVertexShader ) )
    {
        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        return FALSE;
    }
    if( !FrmLoadShaderObjectFromFile( strFragmentShaderFileName, hFragmentShader ) )
    {
        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        return FALSE;
    }

    // Attach the individual shaders to the common shader program
    GLuint hShaderProgram = glCreateProgram();
    glAttachShader( hShaderProgram, hVertexShader );
    glAttachShader( hShaderProgram, hFragmentShader );

    // Bind the attributes to the shader program
    FrmBindShaderAttributes( hShaderProgram, pAttributes, nNumAttributes );

    // Link the vertex shader and fragment shader together
    if( FALSE == FrmLinkShaderProgram( hShaderProgram ) )
    {
        glDeleteProgram( hShaderProgram );
        return FALSE;
    }

    // It's now safe to toss away the original shader handles
    glDeleteShader( hVertexShader );
    glDeleteShader( hFragmentShader );

    // Return the shader program
    (*pShaderProgramHandle) = hShaderProgram;
    return TRUE;
}

/*
//--------------------------------------------------------------------------------------
// Name: FrmLoadShaderProgramFromFile()
// Desc: 
//--------------------------------------------------------------------------------------
BOOL FrmLoadShaderProgramFromFile( const CHAR* strBinaryShaderFileName,
                                   GLuint* pShaderProgramHandle,
                                   const FRM_SHADER_ATTRIBUTE* pAttributes,
                                   UINT32 nNumAttributes )
{
    // Load the shader source
    CHAR* strShaderSource;
    UINT32 nShaderLength;
    if( FALSE == FrmLoadFile( strBinaryShaderFileName, (VOID**)&strShaderSource,
                              &nShaderLength ) )
    {
        FrmLogMessage( "ERROR: Could not load shader file '", strBinaryShaderFileName, "'\n" );
        return FALSE;
    }

    // Create the object handles
    GLuint hVertexShader   = glCreateShader( GL_VERTEX_SHADER );
    GLuint hFragmentShader = glCreateShader( GL_FRAGMENT_SHADER );
    GLuint shaders[2] = { hVertexShader, hFragmentShader };

    // Compile the shader
    glShaderBinary( 2, shaders, GL_BINARY_FORMAT_OPENGL_ES_2_0_AMD_EMULATOR,
                    strShaderSource, nShaderLength );
    FrmUnloadFile( strShaderSource );

    // Check for compile success
    GLint nCompileResult1 = 0;
    GLint nCompileResult2 = 0;
    glGetShaderiv( hVertexShader, GL_COMPILE_STATUS, &nCompileResult1 );
    glGetShaderiv( hFragmentShader, GL_COMPILE_STATUS, &nCompileResult2 );
    if( 0 == nCompileResult1 || 0 == nCompileResult2 )
    {
        CHAR strInfoLog[1024];
        GLint nLength;
        glGetShaderInfoLog( hVertexShader, 1024, &nLength, strInfoLog );
        if( strInfoLog[0] ) FrmLogMessage( strInfoLog, "\n" );
        glGetShaderInfoLog( hFragmentShader, 1024, &nLength, strInfoLog );
        if( strInfoLog[0] ) FrmLogMessage( strInfoLog, "\n" );
        glDeleteShader( hVertexShader );
        glDeleteShader( hFragmentShader );
        return FALSE;
    }

    // Attach the individual shaders to the common shader program
    GLuint hShaderProgram  = glCreateProgram();
    glAttachShader( hShaderProgram, hVertexShader );
    glAttachShader( hShaderProgram, hFragmentShader );

    // Bind the attributes to the shader program
    FrmBindShaderAttributes( hShaderProgram, pAttributes, nNumAttributes );
    
    // Link the vertex shader and fragment shader together
    if( FALSE == FrmLinkShaderProgram( hShaderProgram ) )
    {
        glDeleteProgram( hShaderProgram );
        return FALSE;
    }

    // It's now safe to toss away the original shader handles
    glDeleteShader( hVertexShader );
    glDeleteShader( hFragmentShader );

    // Return the shader program
    (*pShaderProgramHandle) = hShaderProgram;
    return TRUE;
}

*/