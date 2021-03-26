// Copyright (c) 2021, Qualcomm Innovation Center, Inc. All rights reserved.
// SPDX-License-Identifier: BSD-3-Clause

#include "FrmGLES3.h"



#if defined(ANDROID)

#include <android/log.h>

#endif



// For vprintf()

#include <stdio.h>

#include <stdlib.h>

#include <stdarg.h>





/* OpenGL ES 2.0 Extensions */



PFNGLEGLIMAGETARGETTEXTURE2DOESPROC				glEGLImageTargetTexture2DOES			= NULL;

PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC	glEGLImageTargetRenderbufferStorageOES	= NULL;

PFNGLGETPROGRAMBINARYOESPROC					glGetProgramBinaryOES					= NULL;

PFNGLPROGRAMBINARYOESPROC						glProgramBinaryOES						= NULL;

PFNGLMAPBUFFEROESPROC							glMapBufferOES							= NULL;

PFNGLUNMAPBUFFEROESPROC							glUnmapBufferOES						= NULL;

PFNGLGETBUFFERPOINTERVOESPROC					glGetBufferPointervOES					= NULL;

PFNGLTEXIMAGE3DOESPROC							glTexImage3DOES							= NULL;

PFNGLTEXSUBIMAGE3DOESPROC						glTexSubImage3DOES						= NULL;

PFNGLCOPYTEXSUBIMAGE3DOESPROC					glCopyTexSubImage3DOES					= NULL;

PFNGLCOMPRESSEDTEXIMAGE3DOESPROC				glCompressedTexImage3DOES				= NULL;

PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC				glCompressedTexSubImage3DOES			= NULL;

PFNGLFRAMEBUFFERTEXTURE3DOESPROC				glFramebufferTexture3DOES				= NULL;

PFNGLBINDVERTEXARRAYOESPROC						glBindVertexArrayOES					= NULL;

PFNGLDELETEVERTEXARRAYSOESPROC					glDeleteVertexArraysOES					= NULL;

PFNGLGENVERTEXARRAYSOESPROC						glGenVertexArraysOES					= NULL;

PFNGLISVERTEXARRAYOESPROC						glIsVertexArrayOES						= NULL;

PFNGLDEBUGMESSAGECONTROLKHRPROC					glDebugMessageControl					= NULL;

PFNGLDEBUGMESSAGEINSERTKHRPROC						glDebugMessageInsert					= NULL;

PFNGLDEBUGMESSAGECALLBACKKHRPROC					glDebugMessageCallback					= NULL;

PFNGLGETDEBUGMESSAGELOGKHRPROC						glGetDebugMessageLog					= NULL;

PFNGLPUSHDEBUGGROUPKHRPROC							glPushDebugGroup						= NULL;

PFNGLPOPDEBUGGROUPKHRPROC							glPopDebugGroup							= NULL;

PFNGLOBJECTLABELKHRPROC							glObjectLabel							= NULL;

PFNGLGETOBJECTLABELKHRPROC							glGetObjectLabel						= NULL;

PFNGLOBJECTPTRLABELKHRPROC							glObjectPtrLabel						= NULL;

PFNGLGETOBJECTPTRLABELKHRPROC						glGetObjectPtrLabel						= NULL;

PFNGLGETPOINTERVKHRPROC							glGetPointerv							= NULL;

PFNGLGETPERFMONITORGROUPSAMDPROC				glGetPerfMonitorGroupsAMD				= NULL;

PFNGLGETPERFMONITORCOUNTERSAMDPROC				glGetPerfMonitorCountersAMD				= NULL;

PFNGLGETPERFMONITORGROUPSTRINGAMDPROC			glGetPerfMonitorGroupStringAMD			= NULL;

PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC			glGetPerfMonitorCounterStringAMD		= NULL;

PFNGLGETPERFMONITORCOUNTERINFOAMDPROC			glGetPerfMonitorCounterInfoAMD			= NULL;

PFNGLGENPERFMONITORSAMDPROC						glGenPerfMonitorsAMD					= NULL;

PFNGLDELETEPERFMONITORSAMDPROC					glDeletePerfMonitorsAMD					= NULL;

PFNGLSELECTPERFMONITORCOUNTERSAMDPROC			glSelectPerfMonitorCountersAMD			= NULL;

PFNGLBEGINPERFMONITORAMDPROC					glBeginPerfMonitorAMD					= NULL;

PFNGLENDPERFMONITORAMDPROC						glEndPerfMonitorAMD						= NULL;

PFNGLGETPERFMONITORCOUNTERDATAAMDPROC			glGetPerfMonitorCounterDataAMD			= NULL;

PFNGLBLITFRAMEBUFFERANGLEPROC					glBlitFramebufferANGLE					= NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC	glRenderbufferStorageMultisampleANGLE	= NULL;

PFNGLDRAWARRAYSINSTANCEDANGLEPROC				glDrawArraysInstancedANGLE				= NULL;

PFNGLDRAWELEMENTSINSTANCEDANGLEPROC				glDrawElementsInstancedANGLE			= NULL;

PFNGLVERTEXATTRIBDIVISORANGLEPROC				glVertexAttribDivisorANGLE				= NULL;

PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC			glGetTranslatedShaderSourceANGLE		= NULL;

PFNGLCOPYTEXTURELEVELSAPPLEPROC					glCopyTextureLevelsAPPLE				= NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC	glRenderbufferStorageMultisampleAPPLE	= NULL;

PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC		glResolveMultisampleFramebufferAPPLE	= NULL;

PFNGLFENCESYNCAPPLEPROC							glFenceSyncAPPLE						= NULL;

PFNGLISSYNCAPPLEPROC							glIsSyncAPPLE							= NULL;

PFNGLDELETESYNCAPPLEPROC						glDeleteSyncAPPLE						= NULL;

PFNGLCLIENTWAITSYNCAPPLEPROC					glClientWaitSyncAPPLE					= NULL;

PFNGLWAITSYNCAPPLEPROC							glWaitSyncAPPLE							= NULL;

PFNGLGETINTEGER64VAPPLEPROC						glGetInteger64vAPPLE					= NULL;

PFNGLGETSYNCIVAPPLEPROC							glGetSyncivAPPLE						= NULL;

PFNGLLABELOBJECTEXTPROC							glLabelObjectEXT						= NULL;

PFNGLGETOBJECTLABELEXTPROC						glGetObjectLabelEXT						= NULL;

PFNGLINSERTEVENTMARKEREXTPROC					glInsertEventMarkerEXT					= NULL;

PFNGLPUSHGROUPMARKEREXTPROC						glPushGroupMarkerEXT					= NULL;

PFNGLPOPGROUPMARKEREXTPROC						glPopGroupMarkerEXT						= NULL;

PFNGLDISCARDFRAMEBUFFEREXTPROC					glDiscardFramebufferEXT					= NULL;

PFNGLMAPBUFFERRANGEEXTPROC						glMapBufferRangeEXT						= NULL;

PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC				glFlushMappedBufferRangeEXT				= NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC		glRenderbufferStorageMultisampleEXT		= NULL;

PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC		glFramebufferTexture2DMultisampleEXT	= NULL;

PFNGLREADBUFFERINDEXEDEXTPROC					glReadBufferIndexedEXT					= NULL;

PFNGLDRAWBUFFERSINDEXEDEXTPROC					glDrawBuffersIndexedEXT					= NULL;

PFNGLGETINTEGERI_VEXTPROC						glGetIntegeri_vEXT						= NULL;

PFNGLMULTIDRAWARRAYSEXTPROC						glMultiDrawArraysEXT					= NULL;

PFNGLMULTIDRAWELEMENTSEXTPROC					glMultiDrawElementsEXT					= NULL;

PFNGLGENQUERIESEXTPROC							glGenQueriesEXT							= NULL;

PFNGLDELETEQUERIESEXTPROC						glDeleteQueriesEXT						= NULL;

PFNGLISQUERYEXTPROC								glIsQueryEXT							= NULL;

PFNGLBEGINQUERYEXTPROC							glBeginQueryEXT							= NULL;

PFNGLENDQUERYEXTPROC							glEndQueryEXT							= NULL;

PFNGLGETQUERYIVEXTPROC							glGetQueryivEXT							= NULL;

PFNGLGETQUERYOBJECTUIVEXTPROC					glGetQueryObjectuivEXT					= NULL;

PFNGLGETGRAPHICSRESETSTATUSEXTPROC				glGetGraphicsResetStatusEXT				= NULL;

PFNGLREADNPIXELSEXTPROC							glReadnPixelsEXT						= NULL;

PFNGLGETNUNIFORMFVEXTPROC						glGetnUniformfvEXT						= NULL;

PFNGLGETNUNIFORMIVEXTPROC						glGetnUniformivEXT						= NULL;

PFNGLUSEPROGRAMSTAGESEXTPROC					glUseProgramStagesEXT					= NULL;

PFNGLACTIVESHADERPROGRAMEXTPROC					glActiveShaderProgramEXT				= NULL;

PFNGLCREATESHADERPROGRAMVEXTPROC				glCreateShaderProgramvEXT				= NULL;

PFNGLBINDPROGRAMPIPELINEEXTPROC					glBindProgramPipelineEXT				= NULL;

PFNGLDELETEPROGRAMPIPELINESEXTPROC				glDeleteProgramPipelinesEXT				= NULL;

PFNGLGENPROGRAMPIPELINESEXTPROC					glGenProgramPipelinesEXT				= NULL;

PFNGLISPROGRAMPIPELINEEXTPROC					glIsProgramPipelineEXT					= NULL;

PFNGLPROGRAMPARAMETERIEXTPROC					glProgramParameteriEXT					= NULL;

PFNGLGETPROGRAMPIPELINEIVEXTPROC				glGetProgramPipelineivEXT				= NULL;

PFNGLPROGRAMUNIFORM1IEXTPROC					glProgramUniform1iEXT					= NULL;

PFNGLPROGRAMUNIFORM2IEXTPROC					glProgramUniform2iEXT					= NULL;

PFNGLPROGRAMUNIFORM3IEXTPROC					glProgramUniform3iEXT					= NULL;

PFNGLPROGRAMUNIFORM4IEXTPROC					glProgramUniform4iEXT					= NULL;

PFNGLPROGRAMUNIFORM1FEXTPROC					glProgramUniform1fEXT					= NULL;

PFNGLPROGRAMUNIFORM2FEXTPROC					glProgramUniform2fEXT					= NULL;

PFNGLPROGRAMUNIFORM3FEXTPROC					glProgramUniform3fEXT					= NULL;

PFNGLPROGRAMUNIFORM4FEXTPROC					glProgramUniform4fEXT					= NULL;

PFNGLPROGRAMUNIFORM1IVEXTPROC					glProgramUniform1ivEXT					= NULL;

PFNGLPROGRAMUNIFORM2IVEXTPROC					glProgramUniform2ivEXT					= NULL;

PFNGLPROGRAMUNIFORM3IVEXTPROC					glProgramUniform3ivEXT					= NULL;

PFNGLPROGRAMUNIFORM4IVEXTPROC					glProgramUniform4ivEXT					= NULL;

PFNGLPROGRAMUNIFORM1FVEXTPROC					glProgramUniform1fvEXT					= NULL;

PFNGLPROGRAMUNIFORM2FVEXTPROC					glProgramUniform2fvEXT					= NULL;

PFNGLPROGRAMUNIFORM3FVEXTPROC					glProgramUniform3fvEXT					= NULL;

PFNGLPROGRAMUNIFORM4FVEXTPROC					glProgramUniform4fvEXT					= NULL;

PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC				glProgramUniformMatrix2fvEXT			= NULL;

PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC				glProgramUniformMatrix3fvEXT			= NULL;

PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC				glProgramUniformMatrix4fvEXT			= NULL;

PFNGLVALIDATEPROGRAMPIPELINEEXTPROC				glValidateProgramPipelineEXT			= NULL;

PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC			glGetProgramPipelineInfoLogEXT			= NULL;

PFNGLTEXSTORAGE1DEXTPROC						glTexStorage1DEXT						= NULL;

PFNGLTEXSTORAGE2DEXTPROC						glTexStorage2DEXT						= NULL;

PFNGLTEXSTORAGE3DEXTPROC						glTexStorage3DEXT						= NULL;

PFNGLTEXTURESTORAGE1DEXTPROC					glTextureStorage1DEXT					= NULL;

PFNGLTEXTURESTORAGE2DEXTPROC					glTextureStorage2DEXT					= NULL;

PFNGLTEXTURESTORAGE3DEXTPROC					glTextureStorage3DEXT					= NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC		glRenderbufferStorageMultisampleIMG		= NULL;

PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC		glFramebufferTexture2DMultisampleIMG	= NULL;

PFNGLCOVERAGEMASKNVPROC							glCoverageMaskNV						= NULL;

PFNGLCOVERAGEOPERATIONNVPROC					glCoverageOperationNV					= NULL;

PFNGLDRAWBUFFERSNVPROC							glDrawBuffersNV							= NULL;

PFNGLDRAWARRAYSINSTANCEDNVPROC					glDrawArraysInstancedNV					= NULL;

PFNGLDRAWELEMENTSINSTANCEDNVPROC				glDrawElementsInstancedNV				= NULL;

PFNGLDELETEFENCESNVPROC							glDeleteFencesNV						= NULL;

PFNGLGENFENCESNVPROC							glGenFencesNV							= NULL;

PFNGLISFENCENVPROC								glIsFenceNV								= NULL;

PFNGLTESTFENCENVPROC							glTestFenceNV							= NULL;

PFNGLGETFENCEIVNVPROC							glGetFenceivNV							= NULL;

PFNGLFINISHFENCENVPROC							glFinishFenceNV							= NULL;

PFNGLSETFENCENVPROC								glSetFenceNV							= NULL;

PFNGLBLITFRAMEBUFFERNVPROC						glBlitFramebufferNV						= NULL;

PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC		glRenderbufferStorageMultisampleNV		= NULL;

PFNGLVERTEXATTRIBDIVISORNVPROC					glVertexAttribDivisorNV					= NULL;

PFNGLREADBUFFERNVPROC							glReadBufferNV							= NULL;

PFNGLALPHAFUNCQCOMPROC							glAlphaFuncQCOM							= NULL;

PFNGLGETDRIVERCONTROLSQCOMPROC					glGetDriverControlsQCOM					= NULL;

PFNGLGETDRIVERCONTROLSTRINGQCOMPROC				glGetDriverControlStringQCOM			= NULL;

PFNGLENABLEDRIVERCONTROLQCOMPROC				glEnableDriverControlQCOM				= NULL;

PFNGLDISABLEDRIVERCONTROLQCOMPROC				glDisableDriverControlQCOM				= NULL;

PFNGLEXTGETTEXTURESQCOMPROC						glExtGetTexturesQCOM					= NULL;

PFNGLEXTGETBUFFERSQCOMPROC						glExtGetBuffersQCOM						= NULL;

PFNGLEXTGETRENDERBUFFERSQCOMPROC				glExtGetRenderbuffersQCOM				= NULL;

PFNGLEXTGETFRAMEBUFFERSQCOMPROC					glExtGetFramebuffersQCOM				= NULL;

PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC			glExtGetTexLevelParameterivQCOM			= NULL;

PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC			glExtTexObjectStateOverrideiQCOM		= NULL;

PFNGLEXTGETTEXSUBIMAGEQCOMPROC					glExtGetTexSubImageQCOM					= NULL;

PFNGLEXTGETBUFFERPOINTERVQCOMPROC				glExtGetBufferPointervQCOM				= NULL;

PFNGLEXTGETSHADERSQCOMPROC						glExtGetShadersQCOM						= NULL;

PFNGLEXTGETPROGRAMSQCOMPROC						glExtGetProgramsQCOM					= NULL;

PFNGLEXTISPROGRAMBINARYQCOMPROC					glExtIsProgramBinaryQCOM				= NULL;

PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC			glExtGetProgramBinarySourceQCOM			= NULL;

PFNGLSTARTTILINGQCOMPROC						glStartTilingQCOM						= NULL;

PFNGLENDTILINGQCOMPROC							glEndTilingQCOM							= NULL;



//////////////////////////////////////////////////////////////////////////



void OutputLog(const char* pszMessage, ...)

{

    va_list args;

    va_start(args, pszMessage);



#if defined(ANDROID)

    __android_log_vprint(ANDROID_LOG_INFO, "GLES3", pszMessage, args);

#else

    vprintf(pszMessage, args);

#endif



    va_end(args);

}



//////////////////////////////////////////////////////////////////////////



fpnGenericFunctionPointer GetExtension(const char* pszExtensionName)

{

    fpnGenericFunctionPointer fpnExtension = eglGetProcAddress(pszExtensionName);



    if (fpnExtension == NULL)

    {

        OutputLog("The function '%s' is not supported by the OpenGL driver on this device\n", pszExtensionName);

    }



    return fpnExtension;

}



//////////////////////////////////////////////////////////////////////////



int RetrieveES2ExtES3Funcs()

{

    /* Check OpenGL ES version */

    char* pszVersion = (char*)glGetString(GL_VERSION);

    OutputLog("OpenGL ES version found: %s\n", pszVersion);



     /* OpenGL ES 2.0 Extensions */



    glEGLImageTargetTexture2DOES			= (PFNGLEGLIMAGETARGETTEXTURE2DOESPROC)GetExtension("glEGLImageTargetTexture2DOES");

    glEGLImageTargetRenderbufferStorageOES	= (PFNGLEGLIMAGETARGETRENDERBUFFERSTORAGEOESPROC)GetExtension("glEGLImageTargetRenderbufferStorageOES");

    glGetProgramBinaryOES					= (PFNGLGETPROGRAMBINARYOESPROC)GetExtension("glGetProgramBinaryOES");

    glProgramBinaryOES						= (PFNGLPROGRAMBINARYOESPROC)GetExtension("glProgramBinaryOES");

    glMapBufferOES							= (PFNGLMAPBUFFEROESPROC)GetExtension("glMapBufferOES");

    glUnmapBufferOES						= (PFNGLUNMAPBUFFEROESPROC)GetExtension("glUnmapBufferOES");

    glGetBufferPointervOES					= (PFNGLGETBUFFERPOINTERVOESPROC)GetExtension("glGetBufferPointervOES");

    glTexImage3DOES							= (PFNGLTEXIMAGE3DOESPROC)GetExtension("glTexImage3DOES");

    glTexSubImage3DOES						= (PFNGLTEXSUBIMAGE3DOESPROC)GetExtension("glTexSubImage3DOES");

    glCopyTexSubImage3DOES					= (PFNGLCOPYTEXSUBIMAGE3DOESPROC)GetExtension("glCopyTexSubImage3DOES");

    glCompressedTexImage3DOES				= (PFNGLCOMPRESSEDTEXIMAGE3DOESPROC)GetExtension("glCompressedTexImage3DOES");

    glCompressedTexSubImage3DOES			= (PFNGLCOMPRESSEDTEXSUBIMAGE3DOESPROC)GetExtension("glCompressedTexSubImage3DOES");

    glFramebufferTexture3DOES				= (PFNGLFRAMEBUFFERTEXTURE3DOESPROC)GetExtension("glFramebufferTexture3DOES");

    glBindVertexArrayOES					= (PFNGLBINDVERTEXARRAYOESPROC)GetExtension("glBindVertexArrayOES");

    glDeleteVertexArraysOES					= (PFNGLDELETEVERTEXARRAYSOESPROC)GetExtension("glDeleteVertexArraysOES");

    glGenVertexArraysOES					= (PFNGLGENVERTEXARRAYSOESPROC)GetExtension("glGenVertexArraysOES");

    glIsVertexArrayOES						= (PFNGLISVERTEXARRAYOESPROC)GetExtension("glIsVertexArrayOES");

    glDebugMessageControl					= (PFNGLDEBUGMESSAGECONTROLKHRPROC)GetExtension("glDebugMessageControl");

    glDebugMessageInsert					= (PFNGLDEBUGMESSAGEINSERTKHRPROC)GetExtension("glDebugMessageInsert");

    glDebugMessageCallback					= (PFNGLDEBUGMESSAGECALLBACKKHRPROC)GetExtension("glDebugMessageCallback");

    glGetDebugMessageLog					= (PFNGLGETDEBUGMESSAGELOGKHRPROC)GetExtension("glGetDebugMessageLog");

    glPushDebugGroup						= (PFNGLPUSHDEBUGGROUPKHRPROC)GetExtension("glPushDebugGroup");

    glPopDebugGroup							= (PFNGLPOPDEBUGGROUPKHRPROC)GetExtension("glPopDebugGroup");

    glObjectLabel							= (PFNGLOBJECTLABELKHRPROC)GetExtension("glObjectLabel");

    glGetObjectLabel						= (PFNGLGETOBJECTLABELKHRPROC)GetExtension("glGetObjectLabel");

    glObjectPtrLabel						= (PFNGLOBJECTPTRLABELKHRPROC)GetExtension("glObjectPtrLabel");

    glGetObjectPtrLabel						= (PFNGLGETOBJECTPTRLABELKHRPROC)GetExtension("glGetObjectPtrLabel");

    glGetPointerv							= (PFNGLGETPOINTERVKHRPROC)GetExtension("glGetPointerv");

    glGetPerfMonitorGroupsAMD				= (PFNGLGETPERFMONITORGROUPSAMDPROC)GetExtension("glGetPerfMonitorGroupsAMD");

    glGetPerfMonitorCountersAMD				= (PFNGLGETPERFMONITORCOUNTERSAMDPROC)GetExtension("glGetPerfMonitorCountersAMD");

    glGetPerfMonitorGroupStringAMD			= (PFNGLGETPERFMONITORGROUPSTRINGAMDPROC)GetExtension("glGetPerfMonitorGroupStringAMD");

    glGetPerfMonitorCounterStringAMD		= (PFNGLGETPERFMONITORCOUNTERSTRINGAMDPROC)GetExtension("glGetPerfMonitorCounterStringAMD");

    glGetPerfMonitorCounterInfoAMD			= (PFNGLGETPERFMONITORCOUNTERINFOAMDPROC)GetExtension("glGetPerfMonitorCounterInfoAMD");

    glGenPerfMonitorsAMD					= (PFNGLGENPERFMONITORSAMDPROC)GetExtension("glGenPerfMonitorsAMD");

    glDeletePerfMonitorsAMD					= (PFNGLDELETEPERFMONITORSAMDPROC)GetExtension("glDeletePerfMonitorsAMD");

    glSelectPerfMonitorCountersAMD			= (PFNGLSELECTPERFMONITORCOUNTERSAMDPROC)GetExtension("glSelectPerfMonitorCountersAMD");

    glBeginPerfMonitorAMD					= (PFNGLBEGINPERFMONITORAMDPROC)GetExtension("glBeginPerfMonitorAMD");

    glEndPerfMonitorAMD						= (PFNGLENDPERFMONITORAMDPROC)GetExtension("glEndPerfMonitorAMD");

    glGetPerfMonitorCounterDataAMD			= (PFNGLGETPERFMONITORCOUNTERDATAAMDPROC)GetExtension("glGetPerfMonitorCounterDataAMD");

    glBlitFramebufferANGLE					= (PFNGLBLITFRAMEBUFFERANGLEPROC)GetExtension("glBlitFramebufferANGLE");

    glRenderbufferStorageMultisampleANGLE	= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEANGLEPROC)GetExtension("glRenderbufferStorageMultisampleANGLE");

    glDrawArraysInstancedANGLE				= (PFNGLDRAWARRAYSINSTANCEDANGLEPROC)GetExtension("glDrawArraysInstancedANGLE");

    glDrawElementsInstancedANGLE			= (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)GetExtension("glDrawElementsInstancedANGLE");

    glVertexAttribDivisorANGLE				= (PFNGLVERTEXATTRIBDIVISORANGLEPROC)GetExtension("glVertexAttribDivisorANGLE");

    glGetTranslatedShaderSourceANGLE		= (PFNGLGETTRANSLATEDSHADERSOURCEANGLEPROC)GetExtension("glGetTranslatedShaderSourceANGLE");

    glCopyTextureLevelsAPPLE				= (PFNGLCOPYTEXTURELEVELSAPPLEPROC)GetExtension("glCopyTextureLevelsAPPLE");

    glRenderbufferStorageMultisampleAPPLE	= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEAPPLEPROC)GetExtension("glRenderbufferStorageMultisampleAPPLE");

    glResolveMultisampleFramebufferAPPLE	= (PFNGLRESOLVEMULTISAMPLEFRAMEBUFFERAPPLEPROC)GetExtension("glResolveMultisampleFramebufferAPPLE");

    glFenceSyncAPPLE						= (PFNGLFENCESYNCAPPLEPROC)GetExtension("glFenceSyncAPPLE");

    glIsSyncAPPLE							= (PFNGLISSYNCAPPLEPROC)GetExtension("glIsSyncAPPLE");

    glDeleteSyncAPPLE						= (PFNGLDELETESYNCAPPLEPROC)GetExtension("glDeleteSyncAPPLE");

    glClientWaitSyncAPPLE					= (PFNGLCLIENTWAITSYNCAPPLEPROC)GetExtension("glClientWaitSyncAPPLE");

    glWaitSyncAPPLE							= (PFNGLWAITSYNCAPPLEPROC)GetExtension("glWaitSyncAPPLE");

    glGetInteger64vAPPLE					= (PFNGLGETINTEGER64VAPPLEPROC)GetExtension("glGetInteger64vAPPLE");

    glGetSyncivAPPLE						= (PFNGLGETSYNCIVAPPLEPROC)GetExtension("glGetSyncivAPPLE");

    glLabelObjectEXT						= (PFNGLLABELOBJECTEXTPROC)GetExtension("glLabelObjectEXT");

    glGetObjectLabelEXT						= (PFNGLGETOBJECTLABELEXTPROC)GetExtension("glGetObjectLabelEXT");

    glInsertEventMarkerEXT					= (PFNGLINSERTEVENTMARKEREXTPROC)GetExtension("glInsertEventMarkerEXT");

    glPushGroupMarkerEXT					= (PFNGLPUSHGROUPMARKEREXTPROC)GetExtension("glPushGroupMarkerEXT");

    glPopGroupMarkerEXT						= (PFNGLPOPGROUPMARKEREXTPROC)GetExtension("glPopGroupMarkerEXT");

    glDiscardFramebufferEXT					= (PFNGLDISCARDFRAMEBUFFEREXTPROC)GetExtension("glDiscardFramebufferEXT");

    glMapBufferRangeEXT						= (PFNGLMAPBUFFERRANGEEXTPROC)GetExtension("glMapBufferRangeEXT");

    glFlushMappedBufferRangeEXT				= (PFNGLFLUSHMAPPEDBUFFERRANGEEXTPROC)GetExtension("glFlushMappedBufferRangeEXT");

    glRenderbufferStorageMultisampleEXT		= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEEXTPROC)GetExtension("glRenderbufferStorageMultisampleEXT");

    glFramebufferTexture2DMultisampleEXT	= (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEEXTPROC)GetExtension("glFramebufferTexture2DMultisampleEXT");

    glReadBufferIndexedEXT					= (PFNGLREADBUFFERINDEXEDEXTPROC)GetExtension("glReadBufferIndexedEXT");

    glDrawBuffersIndexedEXT					= (PFNGLDRAWBUFFERSINDEXEDEXTPROC)GetExtension("glDrawBuffersIndexedEXT");

    glGetIntegeri_vEXT						= (PFNGLGETINTEGERI_VEXTPROC)GetExtension("glGetIntegeri_vEXT");

    glMultiDrawArraysEXT					= (PFNGLMULTIDRAWARRAYSEXTPROC)GetExtension("glMultiDrawArraysEXT");

    glMultiDrawElementsEXT					= (PFNGLMULTIDRAWELEMENTSEXTPROC)GetExtension("glMultiDrawElementsEXT");

    glGenQueriesEXT							= (PFNGLGENQUERIESEXTPROC)GetExtension("glGenQueriesEXT");

    glDeleteQueriesEXT						= (PFNGLDELETEQUERIESEXTPROC)GetExtension("glDeleteQueriesEXT");

    glIsQueryEXT							= (PFNGLISQUERYEXTPROC)GetExtension("glIsQueryEXT");

    glBeginQueryEXT							= (PFNGLBEGINQUERYEXTPROC)GetExtension("glBeginQueryEXT");

    glEndQueryEXT							= (PFNGLENDQUERYEXTPROC)GetExtension("glEndQueryEXT");

    glGetQueryivEXT							= (PFNGLGETQUERYIVEXTPROC)GetExtension("glGetQueryivEXT");

    glGetQueryObjectuivEXT					= (PFNGLGETQUERYOBJECTUIVEXTPROC)GetExtension("glGetQueryObjectuivEXT");

    glGetGraphicsResetStatusEXT				= (PFNGLGETGRAPHICSRESETSTATUSEXTPROC)GetExtension("glGetGraphicsResetStatusEXT");

    glReadnPixelsEXT						= (PFNGLREADNPIXELSEXTPROC)GetExtension("glReadnPixelsEXT");

    glGetnUniformfvEXT						= (PFNGLGETNUNIFORMFVEXTPROC)GetExtension("glGetnUniformfvEXT");

    glGetnUniformivEXT						= (PFNGLGETNUNIFORMIVEXTPROC)GetExtension("glGetnUniformivEXT");

    glUseProgramStagesEXT					= (PFNGLUSEPROGRAMSTAGESEXTPROC)GetExtension("glUseProgramStagesEXT");

    glActiveShaderProgramEXT				= (PFNGLACTIVESHADERPROGRAMEXTPROC)GetExtension("glActiveShaderProgramEXT");

    glCreateShaderProgramvEXT				= (PFNGLCREATESHADERPROGRAMVEXTPROC)GetExtension("glCreateShaderProgramvEXT");

    glBindProgramPipelineEXT				= (PFNGLBINDPROGRAMPIPELINEEXTPROC)GetExtension("glBindProgramPipelineEXT");

    glDeleteProgramPipelinesEXT				= (PFNGLDELETEPROGRAMPIPELINESEXTPROC)GetExtension("glDeleteProgramPipelinesEXT");

    glGenProgramPipelinesEXT				= (PFNGLGENPROGRAMPIPELINESEXTPROC)GetExtension("glGenProgramPipelinesEXT");

    glIsProgramPipelineEXT					= (PFNGLISPROGRAMPIPELINEEXTPROC)GetExtension("glIsProgramPipelineEXT");

    glProgramParameteriEXT					= (PFNGLPROGRAMPARAMETERIEXTPROC)GetExtension("glProgramParameteriEXT");

    glGetProgramPipelineivEXT				= (PFNGLGETPROGRAMPIPELINEIVEXTPROC)GetExtension("glGetProgramPipelineivEXT");

    glProgramUniform1iEXT					= (PFNGLPROGRAMUNIFORM1IEXTPROC)GetExtension("glProgramUniform1iEXT");

    glProgramUniform2iEXT					= (PFNGLPROGRAMUNIFORM2IEXTPROC)GetExtension("glProgramUniform2iEXT");

    glProgramUniform3iEXT					= (PFNGLPROGRAMUNIFORM3IEXTPROC)GetExtension("glProgramUniform3iEXT");

    glProgramUniform4iEXT					= (PFNGLPROGRAMUNIFORM4IEXTPROC)GetExtension("glProgramUniform4iEXT");

    glProgramUniform1fEXT					= (PFNGLPROGRAMUNIFORM1FEXTPROC)GetExtension("glProgramUniform1fEXT");

    glProgramUniform2fEXT					= (PFNGLPROGRAMUNIFORM2FEXTPROC)GetExtension("glProgramUniform2fEXT");

    glProgramUniform3fEXT					= (PFNGLPROGRAMUNIFORM3FEXTPROC)GetExtension("glProgramUniform3fEXT");

    glProgramUniform4fEXT					= (PFNGLPROGRAMUNIFORM4FEXTPROC)GetExtension("glProgramUniform4fEXT");

    glProgramUniform1ivEXT					= (PFNGLPROGRAMUNIFORM1IVEXTPROC)GetExtension("glProgramUniform1ivEXT");

    glProgramUniform2ivEXT					= (PFNGLPROGRAMUNIFORM2IVEXTPROC)GetExtension("glProgramUniform2ivEXT");

    glProgramUniform3ivEXT					= (PFNGLPROGRAMUNIFORM3IVEXTPROC)GetExtension("glProgramUniform3ivEXT");

    glProgramUniform4ivEXT					= (PFNGLPROGRAMUNIFORM4IVEXTPROC)GetExtension("glProgramUniform4ivEXT");

    glProgramUniform1fvEXT					= (PFNGLPROGRAMUNIFORM1FVEXTPROC)GetExtension("glProgramUniform1fvEXT");

    glProgramUniform2fvEXT					= (PFNGLPROGRAMUNIFORM2FVEXTPROC)GetExtension("glProgramUniform2fvEXT");

    glProgramUniform3fvEXT					= (PFNGLPROGRAMUNIFORM3FVEXTPROC)GetExtension("glProgramUniform3fvEXT");

    glProgramUniform4fvEXT					= (PFNGLPROGRAMUNIFORM4FVEXTPROC)GetExtension("glProgramUniform4fvEXT");

    glProgramUniformMatrix2fvEXT			= (PFNGLPROGRAMUNIFORMMATRIX2FVEXTPROC)GetExtension("glProgramUniformMatrix2fvEXT");

    glProgramUniformMatrix3fvEXT			= (PFNGLPROGRAMUNIFORMMATRIX3FVEXTPROC)GetExtension("glProgramUniformMatrix3fvEXT");

    glProgramUniformMatrix4fvEXT			= (PFNGLPROGRAMUNIFORMMATRIX4FVEXTPROC)GetExtension("glProgramUniformMatrix4fvEXT");

    glValidateProgramPipelineEXT			= (PFNGLVALIDATEPROGRAMPIPELINEEXTPROC)GetExtension("glValidateProgramPipelineEXT");

    glGetProgramPipelineInfoLogEXT			= (PFNGLGETPROGRAMPIPELINEINFOLOGEXTPROC)GetExtension("glGetProgramPipelineInfoLogEXT");

    glTexStorage1DEXT						= (PFNGLTEXSTORAGE1DEXTPROC)GetExtension("glTexStorage1DEXT");

    glTexStorage2DEXT						= (PFNGLTEXSTORAGE2DEXTPROC)GetExtension("glTexStorage2DEXT");

    glTexStorage3DEXT						= (PFNGLTEXSTORAGE3DEXTPROC)GetExtension("glTexStorage3DEXT");

    glTextureStorage1DEXT					= (PFNGLTEXTURESTORAGE1DEXTPROC)GetExtension("glTextureStorage1DEXT");

    glTextureStorage2DEXT					= (PFNGLTEXTURESTORAGE2DEXTPROC)GetExtension("glTextureStorage2DEXT");

    glTextureStorage3DEXT					= (PFNGLTEXTURESTORAGE3DEXTPROC)GetExtension("glTextureStorage3DEXT");

    glRenderbufferStorageMultisampleIMG		= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLEIMGPROC)GetExtension("glRenderbufferStorageMultisampleIMG");

    glFramebufferTexture2DMultisampleIMG	= (PFNGLFRAMEBUFFERTEXTURE2DMULTISAMPLEIMGPROC)GetExtension("glFramebufferTexture2DMultisampleIMG");

    glCoverageMaskNV						= (PFNGLCOVERAGEMASKNVPROC)GetExtension("glCoverageMaskNV");

    glCoverageOperationNV					= (PFNGLCOVERAGEOPERATIONNVPROC)GetExtension("glCoverageOperationNV");

    glDrawBuffersNV							= (PFNGLDRAWBUFFERSNVPROC)GetExtension("glDrawBuffersNV");

    glDrawArraysInstancedNV					= (PFNGLDRAWARRAYSINSTANCEDNVPROC)GetExtension("glDrawArraysInstancedNV");

    glDrawElementsInstancedNV				= (PFNGLDRAWELEMENTSINSTANCEDNVPROC)GetExtension("glDrawElementsInstancedNV");

    glDeleteFencesNV						= (PFNGLDELETEFENCESNVPROC)GetExtension("glDeleteFencesNV");

    glGenFencesNV							= (PFNGLGENFENCESNVPROC)GetExtension("glGenFencesNV");

    glIsFenceNV								= (PFNGLISFENCENVPROC)GetExtension("glIsFenceNV");

    glTestFenceNV							= (PFNGLTESTFENCENVPROC)GetExtension("glTestFenceNV");

    glGetFenceivNV							= (PFNGLGETFENCEIVNVPROC)GetExtension("glGetFenceivNV");

    glFinishFenceNV							= (PFNGLFINISHFENCENVPROC)GetExtension("glFinishFenceNV");

    glSetFenceNV							= (PFNGLSETFENCENVPROC)GetExtension("glSetFenceNV");

    glBlitFramebufferNV						= (PFNGLBLITFRAMEBUFFERNVPROC)GetExtension("glBlitFramebufferNV");

    glRenderbufferStorageMultisampleNV		= (PFNGLRENDERBUFFERSTORAGEMULTISAMPLENVPROC)GetExtension("glRenderbufferStorageMultisampleNV");

    glVertexAttribDivisorNV					= (PFNGLVERTEXATTRIBDIVISORNVPROC)GetExtension("glVertexAttribDivisorNV");

    glReadBufferNV							= (PFNGLREADBUFFERNVPROC)GetExtension("glReadBufferNV");

    glAlphaFuncQCOM							= (PFNGLALPHAFUNCQCOMPROC)GetExtension("glAlphaFuncQCOM");

    glGetDriverControlsQCOM					= (PFNGLGETDRIVERCONTROLSQCOMPROC)GetExtension("glGetDriverControlsQCOM");

    glGetDriverControlStringQCOM			= (PFNGLGETDRIVERCONTROLSTRINGQCOMPROC)GetExtension("glGetDriverControlStringQCOM");

    glEnableDriverControlQCOM				= (PFNGLENABLEDRIVERCONTROLQCOMPROC)GetExtension("glEnableDriverControlQCOM");

    glDisableDriverControlQCOM				= (PFNGLDISABLEDRIVERCONTROLQCOMPROC)GetExtension("glDisableDriverControlQCOM");

    glExtGetTexturesQCOM					= (PFNGLEXTGETTEXTURESQCOMPROC)GetExtension("glExtGetTexturesQCOM");

    glExtGetBuffersQCOM						= (PFNGLEXTGETBUFFERSQCOMPROC)GetExtension("glExtGetBuffersQCOM");

    glExtGetRenderbuffersQCOM				= (PFNGLEXTGETRENDERBUFFERSQCOMPROC)GetExtension("glExtGetRenderbuffersQCOM");

    glExtGetFramebuffersQCOM				= (PFNGLEXTGETFRAMEBUFFERSQCOMPROC)GetExtension("glExtGetFramebuffersQCOM");

    glExtGetTexLevelParameterivQCOM			= (PFNGLEXTGETTEXLEVELPARAMETERIVQCOMPROC)GetExtension("glExtGetTexLevelParameterivQCOM");

    glExtTexObjectStateOverrideiQCOM		= (PFNGLEXTTEXOBJECTSTATEOVERRIDEIQCOMPROC)GetExtension("glExtTexObjectStateOverrideiQCOM");

    glExtGetTexSubImageQCOM					= (PFNGLEXTGETTEXSUBIMAGEQCOMPROC)GetExtension("glExtGetTexSubImageQCOM");

    glExtGetBufferPointervQCOM				= (PFNGLEXTGETBUFFERPOINTERVQCOMPROC)GetExtension("glExtGetBufferPointervQCOM");

    glExtGetShadersQCOM						= (PFNGLEXTGETSHADERSQCOMPROC)GetExtension("glExtGetShadersQCOM");

    glExtGetProgramsQCOM					= (PFNGLEXTGETPROGRAMSQCOMPROC)GetExtension("glExtGetProgramsQCOM");

    glExtIsProgramBinaryQCOM				= (PFNGLEXTISPROGRAMBINARYQCOMPROC)GetExtension("glExtIsProgramBinaryQCOM");

    glExtGetProgramBinarySourceQCOM			= (PFNGLEXTGETPROGRAMBINARYSOURCEQCOMPROC)GetExtension("glExtGetProgramBinarySourceQCOM");

    glStartTilingQCOM						= (PFNGLSTARTTILINGQCOMPROC)GetExtension("glStartTilingQCOM");

    glEndTilingQCOM							= (PFNGLENDTILINGQCOMPROC)GetExtension("glEndTilingQCOM");



    return 1;

}



