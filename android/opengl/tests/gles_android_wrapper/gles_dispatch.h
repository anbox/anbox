/*
* Copyright (C) 2011 The Android Open Source Project
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef _GLES_DISPATCH_H
#define _GLES_DISPATCH_H

#include "gles_proc.h"


struct gles_dispatch {
    glAlphaFunc_t glAlphaFunc;
    glClearColor_t glClearColor;
    glClearDepthf_t glClearDepthf;
    glClipPlanef_t glClipPlanef;
    glColor4f_t glColor4f;
    glDepthRangef_t glDepthRangef;
    glFogf_t glFogf;
    glFogfv_t glFogfv;
    glFrustumf_t glFrustumf;
    glGetClipPlanef_t glGetClipPlanef;
    glGetFloatv_t glGetFloatv;
    glGetLightfv_t glGetLightfv;
    glGetMaterialfv_t glGetMaterialfv;
    glGetTexEnvfv_t glGetTexEnvfv;
    glGetTexParameterfv_t glGetTexParameterfv;
    glLightModelf_t glLightModelf;
    glLightModelfv_t glLightModelfv;
    glLightf_t glLightf;
    glLightfv_t glLightfv;
    glLineWidth_t glLineWidth;
    glLoadMatrixf_t glLoadMatrixf;
    glMaterialf_t glMaterialf;
    glMaterialfv_t glMaterialfv;
    glMultMatrixf_t glMultMatrixf;
    glMultiTexCoord4f_t glMultiTexCoord4f;
    glNormal3f_t glNormal3f;
    glOrthof_t glOrthof;
    glPointParameterf_t glPointParameterf;
    glPointParameterfv_t glPointParameterfv;
    glPointSize_t glPointSize;
    glPolygonOffset_t glPolygonOffset;
    glRotatef_t glRotatef;
    glScalef_t glScalef;
    glTexEnvf_t glTexEnvf;
    glTexEnvfv_t glTexEnvfv;
    glTexParameterf_t glTexParameterf;
    glTexParameterfv_t glTexParameterfv;
    glTranslatef_t glTranslatef;
    glActiveTexture_t glActiveTexture;
    glAlphaFuncx_t glAlphaFuncx;
    glBindBuffer_t glBindBuffer;
    glBindTexture_t glBindTexture;
    glBlendFunc_t glBlendFunc;
    glBufferData_t glBufferData;
    glBufferSubData_t glBufferSubData;
    glClear_t glClear;
    glClearColorx_t glClearColorx;
    glClearDepthx_t glClearDepthx;
    glClearStencil_t glClearStencil;
    glClientActiveTexture_t glClientActiveTexture;
    glClipPlanex_t glClipPlanex;
    glColor4ub_t glColor4ub;
    glColor4x_t glColor4x;
    glColorMask_t glColorMask;
    glColorPointer_t glColorPointer;
    glCompressedTexImage2D_t glCompressedTexImage2D;
    glCompressedTexSubImage2D_t glCompressedTexSubImage2D;
    glCopyTexImage2D_t glCopyTexImage2D;
    glCopyTexSubImage2D_t glCopyTexSubImage2D;
    glCullFace_t glCullFace;
    glDeleteBuffers_t glDeleteBuffers;
    glDeleteTextures_t glDeleteTextures;
    glDepthFunc_t glDepthFunc;
    glDepthMask_t glDepthMask;
    glDepthRangex_t glDepthRangex;
    glDisable_t glDisable;
    glDisableClientState_t glDisableClientState;
    glDrawArrays_t glDrawArrays;
    glDrawElements_t glDrawElements;
    glEnable_t glEnable;
    glEnableClientState_t glEnableClientState;
    glFinish_t glFinish;
    glFlush_t glFlush;
    glFogx_t glFogx;
    glFogxv_t glFogxv;
    glFrontFace_t glFrontFace;
    glFrustumx_t glFrustumx;
    glGetBooleanv_t glGetBooleanv;
    glGetBufferParameteriv_t glGetBufferParameteriv;
    glGetClipPlanex_t glGetClipPlanex;
    glGenBuffers_t glGenBuffers;
    glGenTextures_t glGenTextures;
    glGetError_t glGetError;
    glGetFixedv_t glGetFixedv;
    glGetIntegerv_t glGetIntegerv;
    glGetLightxv_t glGetLightxv;
    glGetMaterialxv_t glGetMaterialxv;
    glGetPointerv_t glGetPointerv;
    glGetString_t glGetString;
    glGetTexEnviv_t glGetTexEnviv;
    glGetTexEnvxv_t glGetTexEnvxv;
    glGetTexParameteriv_t glGetTexParameteriv;
    glGetTexParameterxv_t glGetTexParameterxv;
    glHint_t glHint;
    glIsBuffer_t glIsBuffer;
    glIsEnabled_t glIsEnabled;
    glIsTexture_t glIsTexture;
    glLightModelx_t glLightModelx;
    glLightModelxv_t glLightModelxv;
    glLightx_t glLightx;
    glLightxv_t glLightxv;
    glLineWidthx_t glLineWidthx;
    glLoadIdentity_t glLoadIdentity;
    glLoadMatrixx_t glLoadMatrixx;
    glLogicOp_t glLogicOp;
    glMaterialx_t glMaterialx;
    glMaterialxv_t glMaterialxv;
    glMatrixMode_t glMatrixMode;
    glMultMatrixx_t glMultMatrixx;
    glMultiTexCoord4x_t glMultiTexCoord4x;
    glNormal3x_t glNormal3x;
    glNormalPointer_t glNormalPointer;
    glOrthox_t glOrthox;
    glPixelStorei_t glPixelStorei;
    glPointParameterx_t glPointParameterx;
    glPointParameterxv_t glPointParameterxv;
    glPointSizex_t glPointSizex;
    glPolygonOffsetx_t glPolygonOffsetx;
    glPopMatrix_t glPopMatrix;
    glPushMatrix_t glPushMatrix;
    glReadPixels_t glReadPixels;
    glRotatex_t glRotatex;
    glSampleCoverage_t glSampleCoverage;
    glSampleCoveragex_t glSampleCoveragex;
    glScalex_t glScalex;
    glScissor_t glScissor;
    glShadeModel_t glShadeModel;
    glStencilFunc_t glStencilFunc;
    glStencilMask_t glStencilMask;
    glStencilOp_t glStencilOp;
    glTexCoordPointer_t glTexCoordPointer;
    glTexEnvi_t glTexEnvi;
    glTexEnvx_t glTexEnvx;
    glTexEnviv_t glTexEnviv;
    glTexEnvxv_t glTexEnvxv;
    glTexImage2D_t glTexImage2D;
    glTexParameteri_t glTexParameteri;
    glTexParameterx_t glTexParameterx;
    glTexParameteriv_t glTexParameteriv;
    glTexParameterxv_t glTexParameterxv;
    glTexSubImage2D_t glTexSubImage2D;
    glTranslatex_t glTranslatex;
    glVertexPointer_t glVertexPointer;
    glViewport_t glViewport;
    glPointSizePointerOES_t glPointSizePointerOES;
    glBlendEquationSeparateOES_t glBlendEquationSeparateOES;
    glBlendFuncSeparateOES_t glBlendFuncSeparateOES;
    glBlendEquationOES_t glBlendEquationOES;
    glDrawTexsOES_t glDrawTexsOES;
    glDrawTexiOES_t glDrawTexiOES;
    glDrawTexxOES_t glDrawTexxOES;
    glDrawTexsvOES_t glDrawTexsvOES;
    glDrawTexivOES_t glDrawTexivOES;
    glDrawTexxvOES_t glDrawTexxvOES;
    glDrawTexfOES_t glDrawTexfOES;
    glDrawTexfvOES_t glDrawTexfvOES;
    glEGLImageTargetTexture2DOES_t glEGLImageTargetTexture2DOES;
    glEGLImageTargetRenderbufferStorageOES_t glEGLImageTargetRenderbufferStorageOES;
    glAlphaFuncxOES_t glAlphaFuncxOES;
    glClearColorxOES_t glClearColorxOES;
    glClearDepthxOES_t glClearDepthxOES;
    glClipPlanexOES_t glClipPlanexOES;
    glColor4xOES_t glColor4xOES;
    glDepthRangexOES_t glDepthRangexOES;
    glFogxOES_t glFogxOES;
    glFogxvOES_t glFogxvOES;
    glFrustumxOES_t glFrustumxOES;
    glGetClipPlanexOES_t glGetClipPlanexOES;
    glGetFixedvOES_t glGetFixedvOES;
    glGetLightxvOES_t glGetLightxvOES;
    glGetMaterialxvOES_t glGetMaterialxvOES;
    glGetTexEnvxvOES_t glGetTexEnvxvOES;
    glGetTexParameterxvOES_t glGetTexParameterxvOES;
    glLightModelxOES_t glLightModelxOES;
    glLightModelxvOES_t glLightModelxvOES;
    glLightxOES_t glLightxOES;
    glLightxvOES_t glLightxvOES;
    glLineWidthxOES_t glLineWidthxOES;
    glLoadMatrixxOES_t glLoadMatrixxOES;
    glMaterialxOES_t glMaterialxOES;
    glMaterialxvOES_t glMaterialxvOES;
    glMultMatrixxOES_t glMultMatrixxOES;
    glMultiTexCoord4xOES_t glMultiTexCoord4xOES;
    glNormal3xOES_t glNormal3xOES;
    glOrthoxOES_t glOrthoxOES;
    glPointParameterxOES_t glPointParameterxOES;
    glPointParameterxvOES_t glPointParameterxvOES;
    glPointSizexOES_t glPointSizexOES;
    glPolygonOffsetxOES_t glPolygonOffsetxOES;
    glRotatexOES_t glRotatexOES;
    glSampleCoveragexOES_t glSampleCoveragexOES;
    glScalexOES_t glScalexOES;
    glTexEnvxOES_t glTexEnvxOES;
    glTexEnvxvOES_t glTexEnvxvOES;
    glTexParameterxOES_t glTexParameterxOES;
    glTexParameterxvOES_t glTexParameterxvOES;
    glTranslatexOES_t glTranslatexOES;
    glIsRenderbufferOES_t glIsRenderbufferOES;
    glBindRenderbufferOES_t glBindRenderbufferOES;
    glDeleteRenderbuffersOES_t glDeleteRenderbuffersOES;
    glGenRenderbuffersOES_t glGenRenderbuffersOES;
    glRenderbufferStorageOES_t glRenderbufferStorageOES;
    glGetRenderbufferParameterivOES_t glGetRenderbufferParameterivOES;
    glIsFramebufferOES_t glIsFramebufferOES;
    glBindFramebufferOES_t glBindFramebufferOES;
    glDeleteFramebuffersOES_t glDeleteFramebuffersOES;
    glGenFramebuffersOES_t glGenFramebuffersOES;
    glCheckFramebufferStatusOES_t glCheckFramebufferStatusOES;
    glFramebufferRenderbufferOES_t glFramebufferRenderbufferOES;
    glFramebufferTexture2DOES_t glFramebufferTexture2DOES;
    glGetFramebufferAttachmentParameterivOES_t glGetFramebufferAttachmentParameterivOES;
    glGenerateMipmapOES_t glGenerateMipmapOES;
    glMapBufferOES_t glMapBufferOES;
    glUnmapBufferOES_t glUnmapBufferOES;
    glGetBufferPointervOES_t glGetBufferPointervOES;
    glCurrentPaletteMatrixOES_t glCurrentPaletteMatrixOES;
    glLoadPaletteFromModelViewMatrixOES_t glLoadPaletteFromModelViewMatrixOES;
    glMatrixIndexPointerOES_t glMatrixIndexPointerOES;
    glWeightPointerOES_t glWeightPointerOES;
    glQueryMatrixxOES_t glQueryMatrixxOES;
    glDepthRangefOES_t glDepthRangefOES;
    glFrustumfOES_t glFrustumfOES;
    glOrthofOES_t glOrthofOES;
    glClipPlanefOES_t glClipPlanefOES;
    glGetClipPlanefOES_t glGetClipPlanefOES;
    glClearDepthfOES_t glClearDepthfOES;
    glTexGenfOES_t glTexGenfOES;
    glTexGenfvOES_t glTexGenfvOES;
    glTexGeniOES_t glTexGeniOES;
    glTexGenivOES_t glTexGenivOES;
    glTexGenxOES_t glTexGenxOES;
    glTexGenxvOES_t glTexGenxvOES;
    glGetTexGenfvOES_t glGetTexGenfvOES;
    glGetTexGenivOES_t glGetTexGenivOES;
    glGetTexGenxvOES_t glGetTexGenxvOES;
    glBindVertexArrayOES_t glBindVertexArrayOES;
    glDeleteVertexArraysOES_t glDeleteVertexArraysOES;
    glGenVertexArraysOES_t glGenVertexArraysOES;
    glIsVertexArrayOES_t glIsVertexArrayOES;
    glDiscardFramebufferEXT_t glDiscardFramebufferEXT;
    glMultiDrawArraysEXT_t glMultiDrawArraysEXT;
    glMultiDrawElementsEXT_t glMultiDrawElementsEXT;
    glClipPlanefIMG_t glClipPlanefIMG;
    glClipPlanexIMG_t glClipPlanexIMG;
    glRenderbufferStorageMultisampleIMG_t glRenderbufferStorageMultisampleIMG;
    glFramebufferTexture2DMultisampleIMG_t glFramebufferTexture2DMultisampleIMG;
    glDeleteFencesNV_t glDeleteFencesNV;
    glGenFencesNV_t glGenFencesNV;
    glIsFenceNV_t glIsFenceNV;
    glTestFenceNV_t glTestFenceNV;
    glGetFenceivNV_t glGetFenceivNV;
    glFinishFenceNV_t glFinishFenceNV;
    glSetFenceNV_t glSetFenceNV;
    glGetDriverControlsQCOM_t glGetDriverControlsQCOM;
    glGetDriverControlStringQCOM_t glGetDriverControlStringQCOM;
    glEnableDriverControlQCOM_t glEnableDriverControlQCOM;
    glDisableDriverControlQCOM_t glDisableDriverControlQCOM;
    glExtGetTexturesQCOM_t glExtGetTexturesQCOM;
    glExtGetBuffersQCOM_t glExtGetBuffersQCOM;
    glExtGetRenderbuffersQCOM_t glExtGetRenderbuffersQCOM;
    glExtGetFramebuffersQCOM_t glExtGetFramebuffersQCOM;
    glExtGetTexLevelParameterivQCOM_t glExtGetTexLevelParameterivQCOM;
    glExtTexObjectStateOverrideiQCOM_t glExtTexObjectStateOverrideiQCOM;
    glExtGetTexSubImageQCOM_t glExtGetTexSubImageQCOM;
    glExtGetBufferPointervQCOM_t glExtGetBufferPointervQCOM;
    glExtGetShadersQCOM_t glExtGetShadersQCOM;
    glExtGetProgramsQCOM_t glExtGetProgramsQCOM;
    glExtIsProgramBinaryQCOM_t glExtIsProgramBinaryQCOM;
    glExtGetProgramBinarySourceQCOM_t glExtGetProgramBinarySourceQCOM;
    glStartTilingQCOM_t glStartTilingQCOM;
    glEndTilingQCOM_t glEndTilingQCOM;
    //Accessors
    glAlphaFunc_t set_glAlphaFunc(glAlphaFunc_t f) { glAlphaFunc_t retval = glAlphaFunc; glAlphaFunc = f; return retval;}
    glClearColor_t set_glClearColor(glClearColor_t f) { glClearColor_t retval = glClearColor; glClearColor = f; return retval;}
    glClearDepthf_t set_glClearDepthf(glClearDepthf_t f) { glClearDepthf_t retval = glClearDepthf; glClearDepthf = f; return retval;}
    glClipPlanef_t set_glClipPlanef(glClipPlanef_t f) { glClipPlanef_t retval = glClipPlanef; glClipPlanef = f; return retval;}
    glColor4f_t set_glColor4f(glColor4f_t f) { glColor4f_t retval = glColor4f; glColor4f = f; return retval;}
    glDepthRangef_t set_glDepthRangef(glDepthRangef_t f) { glDepthRangef_t retval = glDepthRangef; glDepthRangef = f; return retval;}
    glFogf_t set_glFogf(glFogf_t f) { glFogf_t retval = glFogf; glFogf = f; return retval;}
    glFogfv_t set_glFogfv(glFogfv_t f) { glFogfv_t retval = glFogfv; glFogfv = f; return retval;}
    glFrustumf_t set_glFrustumf(glFrustumf_t f) { glFrustumf_t retval = glFrustumf; glFrustumf = f; return retval;}
    glGetClipPlanef_t set_glGetClipPlanef(glGetClipPlanef_t f) { glGetClipPlanef_t retval = glGetClipPlanef; glGetClipPlanef = f; return retval;}
    glGetFloatv_t set_glGetFloatv(glGetFloatv_t f) { glGetFloatv_t retval = glGetFloatv; glGetFloatv = f; return retval;}
    glGetLightfv_t set_glGetLightfv(glGetLightfv_t f) { glGetLightfv_t retval = glGetLightfv; glGetLightfv = f; return retval;}
    glGetMaterialfv_t set_glGetMaterialfv(glGetMaterialfv_t f) { glGetMaterialfv_t retval = glGetMaterialfv; glGetMaterialfv = f; return retval;}
    glGetTexEnvfv_t set_glGetTexEnvfv(glGetTexEnvfv_t f) { glGetTexEnvfv_t retval = glGetTexEnvfv; glGetTexEnvfv = f; return retval;}
    glGetTexParameterfv_t set_glGetTexParameterfv(glGetTexParameterfv_t f) { glGetTexParameterfv_t retval = glGetTexParameterfv; glGetTexParameterfv = f; return retval;}
    glLightModelf_t set_glLightModelf(glLightModelf_t f) { glLightModelf_t retval = glLightModelf; glLightModelf = f; return retval;}
    glLightModelfv_t set_glLightModelfv(glLightModelfv_t f) { glLightModelfv_t retval = glLightModelfv; glLightModelfv = f; return retval;}
    glLightf_t set_glLightf(glLightf_t f) { glLightf_t retval = glLightf; glLightf = f; return retval;}
    glLightfv_t set_glLightfv(glLightfv_t f) { glLightfv_t retval = glLightfv; glLightfv = f; return retval;}
    glLineWidth_t set_glLineWidth(glLineWidth_t f) { glLineWidth_t retval = glLineWidth; glLineWidth = f; return retval;}
    glLoadMatrixf_t set_glLoadMatrixf(glLoadMatrixf_t f) { glLoadMatrixf_t retval = glLoadMatrixf; glLoadMatrixf = f; return retval;}
    glMaterialf_t set_glMaterialf(glMaterialf_t f) { glMaterialf_t retval = glMaterialf; glMaterialf = f; return retval;}
    glMaterialfv_t set_glMaterialfv(glMaterialfv_t f) { glMaterialfv_t retval = glMaterialfv; glMaterialfv = f; return retval;}
    glMultMatrixf_t set_glMultMatrixf(glMultMatrixf_t f) { glMultMatrixf_t retval = glMultMatrixf; glMultMatrixf = f; return retval;}
    glMultiTexCoord4f_t set_glMultiTexCoord4f(glMultiTexCoord4f_t f) { glMultiTexCoord4f_t retval = glMultiTexCoord4f; glMultiTexCoord4f = f; return retval;}
    glNormal3f_t set_glNormal3f(glNormal3f_t f) { glNormal3f_t retval = glNormal3f; glNormal3f = f; return retval;}
    glOrthof_t set_glOrthof(glOrthof_t f) { glOrthof_t retval = glOrthof; glOrthof = f; return retval;}
    glPointParameterf_t set_glPointParameterf(glPointParameterf_t f) { glPointParameterf_t retval = glPointParameterf; glPointParameterf = f; return retval;}
    glPointParameterfv_t set_glPointParameterfv(glPointParameterfv_t f) { glPointParameterfv_t retval = glPointParameterfv; glPointParameterfv = f; return retval;}
    glPointSize_t set_glPointSize(glPointSize_t f) { glPointSize_t retval = glPointSize; glPointSize = f; return retval;}
    glPolygonOffset_t set_glPolygonOffset(glPolygonOffset_t f) { glPolygonOffset_t retval = glPolygonOffset; glPolygonOffset = f; return retval;}
    glRotatef_t set_glRotatef(glRotatef_t f) { glRotatef_t retval = glRotatef; glRotatef = f; return retval;}
    glScalef_t set_glScalef(glScalef_t f) { glScalef_t retval = glScalef; glScalef = f; return retval;}
    glTexEnvf_t set_glTexEnvf(glTexEnvf_t f) { glTexEnvf_t retval = glTexEnvf; glTexEnvf = f; return retval;}
    glTexEnvfv_t set_glTexEnvfv(glTexEnvfv_t f) { glTexEnvfv_t retval = glTexEnvfv; glTexEnvfv = f; return retval;}
    glTexParameterf_t set_glTexParameterf(glTexParameterf_t f) { glTexParameterf_t retval = glTexParameterf; glTexParameterf = f; return retval;}
    glTexParameterfv_t set_glTexParameterfv(glTexParameterfv_t f) { glTexParameterfv_t retval = glTexParameterfv; glTexParameterfv = f; return retval;}
    glTranslatef_t set_glTranslatef(glTranslatef_t f) { glTranslatef_t retval = glTranslatef; glTranslatef = f; return retval;}
    glActiveTexture_t set_glActiveTexture(glActiveTexture_t f) { glActiveTexture_t retval = glActiveTexture; glActiveTexture = f; return retval;}
    glAlphaFuncx_t set_glAlphaFuncx(glAlphaFuncx_t f) { glAlphaFuncx_t retval = glAlphaFuncx; glAlphaFuncx = f; return retval;}
    glBindBuffer_t set_glBindBuffer(glBindBuffer_t f) { glBindBuffer_t retval = glBindBuffer; glBindBuffer = f; return retval;}
    glBindTexture_t set_glBindTexture(glBindTexture_t f) { glBindTexture_t retval = glBindTexture; glBindTexture = f; return retval;}
    glBlendFunc_t set_glBlendFunc(glBlendFunc_t f) { glBlendFunc_t retval = glBlendFunc; glBlendFunc = f; return retval;}
    glBufferData_t set_glBufferData(glBufferData_t f) { glBufferData_t retval = glBufferData; glBufferData = f; return retval;}
    glBufferSubData_t set_glBufferSubData(glBufferSubData_t f) { glBufferSubData_t retval = glBufferSubData; glBufferSubData = f; return retval;}
    glClear_t set_glClear(glClear_t f) { glClear_t retval = glClear; glClear = f; return retval;}
    glClearColorx_t set_glClearColorx(glClearColorx_t f) { glClearColorx_t retval = glClearColorx; glClearColorx = f; return retval;}
    glClearDepthx_t set_glClearDepthx(glClearDepthx_t f) { glClearDepthx_t retval = glClearDepthx; glClearDepthx = f; return retval;}
    glClearStencil_t set_glClearStencil(glClearStencil_t f) { glClearStencil_t retval = glClearStencil; glClearStencil = f; return retval;}
    glClientActiveTexture_t set_glClientActiveTexture(glClientActiveTexture_t f) { glClientActiveTexture_t retval = glClientActiveTexture; glClientActiveTexture = f; return retval;}
    glClipPlanex_t set_glClipPlanex(glClipPlanex_t f) { glClipPlanex_t retval = glClipPlanex; glClipPlanex = f; return retval;}
    glColor4ub_t set_glColor4ub(glColor4ub_t f) { glColor4ub_t retval = glColor4ub; glColor4ub = f; return retval;}
    glColor4x_t set_glColor4x(glColor4x_t f) { glColor4x_t retval = glColor4x; glColor4x = f; return retval;}
    glColorMask_t set_glColorMask(glColorMask_t f) { glColorMask_t retval = glColorMask; glColorMask = f; return retval;}
    glColorPointer_t set_glColorPointer(glColorPointer_t f) { glColorPointer_t retval = glColorPointer; glColorPointer = f; return retval;}
    glCompressedTexImage2D_t set_glCompressedTexImage2D(glCompressedTexImage2D_t f) { glCompressedTexImage2D_t retval = glCompressedTexImage2D; glCompressedTexImage2D = f; return retval;}
    glCompressedTexSubImage2D_t set_glCompressedTexSubImage2D(glCompressedTexSubImage2D_t f) { glCompressedTexSubImage2D_t retval = glCompressedTexSubImage2D; glCompressedTexSubImage2D = f; return retval;}
    glCopyTexImage2D_t set_glCopyTexImage2D(glCopyTexImage2D_t f) { glCopyTexImage2D_t retval = glCopyTexImage2D; glCopyTexImage2D = f; return retval;}
    glCopyTexSubImage2D_t set_glCopyTexSubImage2D(glCopyTexSubImage2D_t f) { glCopyTexSubImage2D_t retval = glCopyTexSubImage2D; glCopyTexSubImage2D = f; return retval;}
    glCullFace_t set_glCullFace(glCullFace_t f) { glCullFace_t retval = glCullFace; glCullFace = f; return retval;}
    glDeleteBuffers_t set_glDeleteBuffers(glDeleteBuffers_t f) { glDeleteBuffers_t retval = glDeleteBuffers; glDeleteBuffers = f; return retval;}
    glDeleteTextures_t set_glDeleteTextures(glDeleteTextures_t f) { glDeleteTextures_t retval = glDeleteTextures; glDeleteTextures = f; return retval;}
    glDepthFunc_t set_glDepthFunc(glDepthFunc_t f) { glDepthFunc_t retval = glDepthFunc; glDepthFunc = f; return retval;}
    glDepthMask_t set_glDepthMask(glDepthMask_t f) { glDepthMask_t retval = glDepthMask; glDepthMask = f; return retval;}
    glDepthRangex_t set_glDepthRangex(glDepthRangex_t f) { glDepthRangex_t retval = glDepthRangex; glDepthRangex = f; return retval;}
    glDisable_t set_glDisable(glDisable_t f) { glDisable_t retval = glDisable; glDisable = f; return retval;}
    glDisableClientState_t set_glDisableClientState(glDisableClientState_t f) { glDisableClientState_t retval = glDisableClientState; glDisableClientState = f; return retval;}
    glDrawArrays_t set_glDrawArrays(glDrawArrays_t f) { glDrawArrays_t retval = glDrawArrays; glDrawArrays = f; return retval;}
    glDrawElements_t set_glDrawElements(glDrawElements_t f) { glDrawElements_t retval = glDrawElements; glDrawElements = f; return retval;}
    glEnable_t set_glEnable(glEnable_t f) { glEnable_t retval = glEnable; glEnable = f; return retval;}
    glEnableClientState_t set_glEnableClientState(glEnableClientState_t f) { glEnableClientState_t retval = glEnableClientState; glEnableClientState = f; return retval;}
    glFinish_t set_glFinish(glFinish_t f) { glFinish_t retval = glFinish; glFinish = f; return retval;}
    glFlush_t set_glFlush(glFlush_t f) { glFlush_t retval = glFlush; glFlush = f; return retval;}
    glFogx_t set_glFogx(glFogx_t f) { glFogx_t retval = glFogx; glFogx = f; return retval;}
    glFogxv_t set_glFogxv(glFogxv_t f) { glFogxv_t retval = glFogxv; glFogxv = f; return retval;}
    glFrontFace_t set_glFrontFace(glFrontFace_t f) { glFrontFace_t retval = glFrontFace; glFrontFace = f; return retval;}
    glFrustumx_t set_glFrustumx(glFrustumx_t f) { glFrustumx_t retval = glFrustumx; glFrustumx = f; return retval;}
    glGetBooleanv_t set_glGetBooleanv(glGetBooleanv_t f) { glGetBooleanv_t retval = glGetBooleanv; glGetBooleanv = f; return retval;}
    glGetBufferParameteriv_t set_glGetBufferParameteriv(glGetBufferParameteriv_t f) { glGetBufferParameteriv_t retval = glGetBufferParameteriv; glGetBufferParameteriv = f; return retval;}
    glGetClipPlanex_t set_glGetClipPlanex(glGetClipPlanex_t f) { glGetClipPlanex_t retval = glGetClipPlanex; glGetClipPlanex = f; return retval;}
    glGenBuffers_t set_glGenBuffers(glGenBuffers_t f) { glGenBuffers_t retval = glGenBuffers; glGenBuffers = f; return retval;}
    glGenTextures_t set_glGenTextures(glGenTextures_t f) { glGenTextures_t retval = glGenTextures; glGenTextures = f; return retval;}
    glGetError_t set_glGetError(glGetError_t f) { glGetError_t retval = glGetError; glGetError = f; return retval;}
    glGetFixedv_t set_glGetFixedv(glGetFixedv_t f) { glGetFixedv_t retval = glGetFixedv; glGetFixedv = f; return retval;}
    glGetIntegerv_t set_glGetIntegerv(glGetIntegerv_t f) { glGetIntegerv_t retval = glGetIntegerv; glGetIntegerv = f; return retval;}
    glGetLightxv_t set_glGetLightxv(glGetLightxv_t f) { glGetLightxv_t retval = glGetLightxv; glGetLightxv = f; return retval;}
    glGetMaterialxv_t set_glGetMaterialxv(glGetMaterialxv_t f) { glGetMaterialxv_t retval = glGetMaterialxv; glGetMaterialxv = f; return retval;}
    glGetPointerv_t set_glGetPointerv(glGetPointerv_t f) { glGetPointerv_t retval = glGetPointerv; glGetPointerv = f; return retval;}
    glGetString_t set_glGetString(glGetString_t f) { glGetString_t retval = glGetString; glGetString = f; return retval;}
    glGetTexEnviv_t set_glGetTexEnviv(glGetTexEnviv_t f) { glGetTexEnviv_t retval = glGetTexEnviv; glGetTexEnviv = f; return retval;}
    glGetTexEnvxv_t set_glGetTexEnvxv(glGetTexEnvxv_t f) { glGetTexEnvxv_t retval = glGetTexEnvxv; glGetTexEnvxv = f; return retval;}
    glGetTexParameteriv_t set_glGetTexParameteriv(glGetTexParameteriv_t f) { glGetTexParameteriv_t retval = glGetTexParameteriv; glGetTexParameteriv = f; return retval;}
    glGetTexParameterxv_t set_glGetTexParameterxv(glGetTexParameterxv_t f) { glGetTexParameterxv_t retval = glGetTexParameterxv; glGetTexParameterxv = f; return retval;}
    glHint_t set_glHint(glHint_t f) { glHint_t retval = glHint; glHint = f; return retval;}
    glIsBuffer_t set_glIsBuffer(glIsBuffer_t f) { glIsBuffer_t retval = glIsBuffer; glIsBuffer = f; return retval;}
    glIsEnabled_t set_glIsEnabled(glIsEnabled_t f) { glIsEnabled_t retval = glIsEnabled; glIsEnabled = f; return retval;}
    glIsTexture_t set_glIsTexture(glIsTexture_t f) { glIsTexture_t retval = glIsTexture; glIsTexture = f; return retval;}
    glLightModelx_t set_glLightModelx(glLightModelx_t f) { glLightModelx_t retval = glLightModelx; glLightModelx = f; return retval;}
    glLightModelxv_t set_glLightModelxv(glLightModelxv_t f) { glLightModelxv_t retval = glLightModelxv; glLightModelxv = f; return retval;}
    glLightx_t set_glLightx(glLightx_t f) { glLightx_t retval = glLightx; glLightx = f; return retval;}
    glLightxv_t set_glLightxv(glLightxv_t f) { glLightxv_t retval = glLightxv; glLightxv = f; return retval;}
    glLineWidthx_t set_glLineWidthx(glLineWidthx_t f) { glLineWidthx_t retval = glLineWidthx; glLineWidthx = f; return retval;}
    glLoadIdentity_t set_glLoadIdentity(glLoadIdentity_t f) { glLoadIdentity_t retval = glLoadIdentity; glLoadIdentity = f; return retval;}
    glLoadMatrixx_t set_glLoadMatrixx(glLoadMatrixx_t f) { glLoadMatrixx_t retval = glLoadMatrixx; glLoadMatrixx = f; return retval;}
    glLogicOp_t set_glLogicOp(glLogicOp_t f) { glLogicOp_t retval = glLogicOp; glLogicOp = f; return retval;}
    glMaterialx_t set_glMaterialx(glMaterialx_t f) { glMaterialx_t retval = glMaterialx; glMaterialx = f; return retval;}
    glMaterialxv_t set_glMaterialxv(glMaterialxv_t f) { glMaterialxv_t retval = glMaterialxv; glMaterialxv = f; return retval;}
    glMatrixMode_t set_glMatrixMode(glMatrixMode_t f) { glMatrixMode_t retval = glMatrixMode; glMatrixMode = f; return retval;}
    glMultMatrixx_t set_glMultMatrixx(glMultMatrixx_t f) { glMultMatrixx_t retval = glMultMatrixx; glMultMatrixx = f; return retval;}
    glMultiTexCoord4x_t set_glMultiTexCoord4x(glMultiTexCoord4x_t f) { glMultiTexCoord4x_t retval = glMultiTexCoord4x; glMultiTexCoord4x = f; return retval;}
    glNormal3x_t set_glNormal3x(glNormal3x_t f) { glNormal3x_t retval = glNormal3x; glNormal3x = f; return retval;}
    glNormalPointer_t set_glNormalPointer(glNormalPointer_t f) { glNormalPointer_t retval = glNormalPointer; glNormalPointer = f; return retval;}
    glOrthox_t set_glOrthox(glOrthox_t f) { glOrthox_t retval = glOrthox; glOrthox = f; return retval;}
    glPixelStorei_t set_glPixelStorei(glPixelStorei_t f) { glPixelStorei_t retval = glPixelStorei; glPixelStorei = f; return retval;}
    glPointParameterx_t set_glPointParameterx(glPointParameterx_t f) { glPointParameterx_t retval = glPointParameterx; glPointParameterx = f; return retval;}
    glPointParameterxv_t set_glPointParameterxv(glPointParameterxv_t f) { glPointParameterxv_t retval = glPointParameterxv; glPointParameterxv = f; return retval;}
    glPointSizex_t set_glPointSizex(glPointSizex_t f) { glPointSizex_t retval = glPointSizex; glPointSizex = f; return retval;}
    glPolygonOffsetx_t set_glPolygonOffsetx(glPolygonOffsetx_t f) { glPolygonOffsetx_t retval = glPolygonOffsetx; glPolygonOffsetx = f; return retval;}
    glPopMatrix_t set_glPopMatrix(glPopMatrix_t f) { glPopMatrix_t retval = glPopMatrix; glPopMatrix = f; return retval;}
    glPushMatrix_t set_glPushMatrix(glPushMatrix_t f) { glPushMatrix_t retval = glPushMatrix; glPushMatrix = f; return retval;}
    glReadPixels_t set_glReadPixels(glReadPixels_t f) { glReadPixels_t retval = glReadPixels; glReadPixels = f; return retval;}
    glRotatex_t set_glRotatex(glRotatex_t f) { glRotatex_t retval = glRotatex; glRotatex = f; return retval;}
    glSampleCoverage_t set_glSampleCoverage(glSampleCoverage_t f) { glSampleCoverage_t retval = glSampleCoverage; glSampleCoverage = f; return retval;}
    glSampleCoveragex_t set_glSampleCoveragex(glSampleCoveragex_t f) { glSampleCoveragex_t retval = glSampleCoveragex; glSampleCoveragex = f; return retval;}
    glScalex_t set_glScalex(glScalex_t f) { glScalex_t retval = glScalex; glScalex = f; return retval;}
    glScissor_t set_glScissor(glScissor_t f) { glScissor_t retval = glScissor; glScissor = f; return retval;}
    glShadeModel_t set_glShadeModel(glShadeModel_t f) { glShadeModel_t retval = glShadeModel; glShadeModel = f; return retval;}
    glStencilFunc_t set_glStencilFunc(glStencilFunc_t f) { glStencilFunc_t retval = glStencilFunc; glStencilFunc = f; return retval;}
    glStencilMask_t set_glStencilMask(glStencilMask_t f) { glStencilMask_t retval = glStencilMask; glStencilMask = f; return retval;}
    glStencilOp_t set_glStencilOp(glStencilOp_t f) { glStencilOp_t retval = glStencilOp; glStencilOp = f; return retval;}
    glTexCoordPointer_t set_glTexCoordPointer(glTexCoordPointer_t f) { glTexCoordPointer_t retval = glTexCoordPointer; glTexCoordPointer = f; return retval;}
    glTexEnvi_t set_glTexEnvi(glTexEnvi_t f) { glTexEnvi_t retval = glTexEnvi; glTexEnvi = f; return retval;}
    glTexEnvx_t set_glTexEnvx(glTexEnvx_t f) { glTexEnvx_t retval = glTexEnvx; glTexEnvx = f; return retval;}
    glTexEnviv_t set_glTexEnviv(glTexEnviv_t f) { glTexEnviv_t retval = glTexEnviv; glTexEnviv = f; return retval;}
    glTexEnvxv_t set_glTexEnvxv(glTexEnvxv_t f) { glTexEnvxv_t retval = glTexEnvxv; glTexEnvxv = f; return retval;}
    glTexImage2D_t set_glTexImage2D(glTexImage2D_t f) { glTexImage2D_t retval = glTexImage2D; glTexImage2D = f; return retval;}
    glTexParameteri_t set_glTexParameteri(glTexParameteri_t f) { glTexParameteri_t retval = glTexParameteri; glTexParameteri = f; return retval;}
    glTexParameterx_t set_glTexParameterx(glTexParameterx_t f) { glTexParameterx_t retval = glTexParameterx; glTexParameterx = f; return retval;}
    glTexParameteriv_t set_glTexParameteriv(glTexParameteriv_t f) { glTexParameteriv_t retval = glTexParameteriv; glTexParameteriv = f; return retval;}
    glTexParameterxv_t set_glTexParameterxv(glTexParameterxv_t f) { glTexParameterxv_t retval = glTexParameterxv; glTexParameterxv = f; return retval;}
    glTexSubImage2D_t set_glTexSubImage2D(glTexSubImage2D_t f) { glTexSubImage2D_t retval = glTexSubImage2D; glTexSubImage2D = f; return retval;}
    glTranslatex_t set_glTranslatex(glTranslatex_t f) { glTranslatex_t retval = glTranslatex; glTranslatex = f; return retval;}
    glVertexPointer_t set_glVertexPointer(glVertexPointer_t f) { glVertexPointer_t retval = glVertexPointer; glVertexPointer = f; return retval;}
    glViewport_t set_glViewport(glViewport_t f) { glViewport_t retval = glViewport; glViewport = f; return retval;}
    glPointSizePointerOES_t set_glPointSizePointerOES(glPointSizePointerOES_t f) { glPointSizePointerOES_t retval = glPointSizePointerOES; glPointSizePointerOES = f; return retval;}
    glBlendEquationSeparateOES_t set_glBlendEquationSeparateOES(glBlendEquationSeparateOES_t f) { glBlendEquationSeparateOES_t retval = glBlendEquationSeparateOES; glBlendEquationSeparateOES = f; return retval;}
    glBlendFuncSeparateOES_t set_glBlendFuncSeparateOES(glBlendFuncSeparateOES_t f) { glBlendFuncSeparateOES_t retval = glBlendFuncSeparateOES; glBlendFuncSeparateOES = f; return retval;}
    glBlendEquationOES_t set_glBlendEquationOES(glBlendEquationOES_t f) { glBlendEquationOES_t retval = glBlendEquationOES; glBlendEquationOES = f; return retval;}
    glDrawTexsOES_t set_glDrawTexsOES(glDrawTexsOES_t f) { glDrawTexsOES_t retval = glDrawTexsOES; glDrawTexsOES = f; return retval;}
    glDrawTexiOES_t set_glDrawTexiOES(glDrawTexiOES_t f) { glDrawTexiOES_t retval = glDrawTexiOES; glDrawTexiOES = f; return retval;}
    glDrawTexxOES_t set_glDrawTexxOES(glDrawTexxOES_t f) { glDrawTexxOES_t retval = glDrawTexxOES; glDrawTexxOES = f; return retval;}
    glDrawTexsvOES_t set_glDrawTexsvOES(glDrawTexsvOES_t f) { glDrawTexsvOES_t retval = glDrawTexsvOES; glDrawTexsvOES = f; return retval;}
    glDrawTexivOES_t set_glDrawTexivOES(glDrawTexivOES_t f) { glDrawTexivOES_t retval = glDrawTexivOES; glDrawTexivOES = f; return retval;}
    glDrawTexxvOES_t set_glDrawTexxvOES(glDrawTexxvOES_t f) { glDrawTexxvOES_t retval = glDrawTexxvOES; glDrawTexxvOES = f; return retval;}
    glDrawTexfOES_t set_glDrawTexfOES(glDrawTexfOES_t f) { glDrawTexfOES_t retval = glDrawTexfOES; glDrawTexfOES = f; return retval;}
    glDrawTexfvOES_t set_glDrawTexfvOES(glDrawTexfvOES_t f) { glDrawTexfvOES_t retval = glDrawTexfvOES; glDrawTexfvOES = f; return retval;}
    glEGLImageTargetTexture2DOES_t set_glEGLImageTargetTexture2DOES(glEGLImageTargetTexture2DOES_t f) { glEGLImageTargetTexture2DOES_t retval = glEGLImageTargetTexture2DOES; glEGLImageTargetTexture2DOES = f; return retval;}
    glEGLImageTargetRenderbufferStorageOES_t set_glEGLImageTargetRenderbufferStorageOES(glEGLImageTargetRenderbufferStorageOES_t f) { glEGLImageTargetRenderbufferStorageOES_t retval = glEGLImageTargetRenderbufferStorageOES; glEGLImageTargetRenderbufferStorageOES = f; return retval;}
    glAlphaFuncxOES_t set_glAlphaFuncxOES(glAlphaFuncxOES_t f) { glAlphaFuncxOES_t retval = glAlphaFuncxOES; glAlphaFuncxOES = f; return retval;}
    glClearColorxOES_t set_glClearColorxOES(glClearColorxOES_t f) { glClearColorxOES_t retval = glClearColorxOES; glClearColorxOES = f; return retval;}
    glClearDepthxOES_t set_glClearDepthxOES(glClearDepthxOES_t f) { glClearDepthxOES_t retval = glClearDepthxOES; glClearDepthxOES = f; return retval;}
    glClipPlanexOES_t set_glClipPlanexOES(glClipPlanexOES_t f) { glClipPlanexOES_t retval = glClipPlanexOES; glClipPlanexOES = f; return retval;}
    glColor4xOES_t set_glColor4xOES(glColor4xOES_t f) { glColor4xOES_t retval = glColor4xOES; glColor4xOES = f; return retval;}
    glDepthRangexOES_t set_glDepthRangexOES(glDepthRangexOES_t f) { glDepthRangexOES_t retval = glDepthRangexOES; glDepthRangexOES = f; return retval;}
    glFogxOES_t set_glFogxOES(glFogxOES_t f) { glFogxOES_t retval = glFogxOES; glFogxOES = f; return retval;}
    glFogxvOES_t set_glFogxvOES(glFogxvOES_t f) { glFogxvOES_t retval = glFogxvOES; glFogxvOES = f; return retval;}
    glFrustumxOES_t set_glFrustumxOES(glFrustumxOES_t f) { glFrustumxOES_t retval = glFrustumxOES; glFrustumxOES = f; return retval;}
    glGetClipPlanexOES_t set_glGetClipPlanexOES(glGetClipPlanexOES_t f) { glGetClipPlanexOES_t retval = glGetClipPlanexOES; glGetClipPlanexOES = f; return retval;}
    glGetFixedvOES_t set_glGetFixedvOES(glGetFixedvOES_t f) { glGetFixedvOES_t retval = glGetFixedvOES; glGetFixedvOES = f; return retval;}
    glGetLightxvOES_t set_glGetLightxvOES(glGetLightxvOES_t f) { glGetLightxvOES_t retval = glGetLightxvOES; glGetLightxvOES = f; return retval;}
    glGetMaterialxvOES_t set_glGetMaterialxvOES(glGetMaterialxvOES_t f) { glGetMaterialxvOES_t retval = glGetMaterialxvOES; glGetMaterialxvOES = f; return retval;}
    glGetTexEnvxvOES_t set_glGetTexEnvxvOES(glGetTexEnvxvOES_t f) { glGetTexEnvxvOES_t retval = glGetTexEnvxvOES; glGetTexEnvxvOES = f; return retval;}
    glGetTexParameterxvOES_t set_glGetTexParameterxvOES(glGetTexParameterxvOES_t f) { glGetTexParameterxvOES_t retval = glGetTexParameterxvOES; glGetTexParameterxvOES = f; return retval;}
    glLightModelxOES_t set_glLightModelxOES(glLightModelxOES_t f) { glLightModelxOES_t retval = glLightModelxOES; glLightModelxOES = f; return retval;}
    glLightModelxvOES_t set_glLightModelxvOES(glLightModelxvOES_t f) { glLightModelxvOES_t retval = glLightModelxvOES; glLightModelxvOES = f; return retval;}
    glLightxOES_t set_glLightxOES(glLightxOES_t f) { glLightxOES_t retval = glLightxOES; glLightxOES = f; return retval;}
    glLightxvOES_t set_glLightxvOES(glLightxvOES_t f) { glLightxvOES_t retval = glLightxvOES; glLightxvOES = f; return retval;}
    glLineWidthxOES_t set_glLineWidthxOES(glLineWidthxOES_t f) { glLineWidthxOES_t retval = glLineWidthxOES; glLineWidthxOES = f; return retval;}
    glLoadMatrixxOES_t set_glLoadMatrixxOES(glLoadMatrixxOES_t f) { glLoadMatrixxOES_t retval = glLoadMatrixxOES; glLoadMatrixxOES = f; return retval;}
    glMaterialxOES_t set_glMaterialxOES(glMaterialxOES_t f) { glMaterialxOES_t retval = glMaterialxOES; glMaterialxOES = f; return retval;}
    glMaterialxvOES_t set_glMaterialxvOES(glMaterialxvOES_t f) { glMaterialxvOES_t retval = glMaterialxvOES; glMaterialxvOES = f; return retval;}
    glMultMatrixxOES_t set_glMultMatrixxOES(glMultMatrixxOES_t f) { glMultMatrixxOES_t retval = glMultMatrixxOES; glMultMatrixxOES = f; return retval;}
    glMultiTexCoord4xOES_t set_glMultiTexCoord4xOES(glMultiTexCoord4xOES_t f) { glMultiTexCoord4xOES_t retval = glMultiTexCoord4xOES; glMultiTexCoord4xOES = f; return retval;}
    glNormal3xOES_t set_glNormal3xOES(glNormal3xOES_t f) { glNormal3xOES_t retval = glNormal3xOES; glNormal3xOES = f; return retval;}
    glOrthoxOES_t set_glOrthoxOES(glOrthoxOES_t f) { glOrthoxOES_t retval = glOrthoxOES; glOrthoxOES = f; return retval;}
    glPointParameterxOES_t set_glPointParameterxOES(glPointParameterxOES_t f) { glPointParameterxOES_t retval = glPointParameterxOES; glPointParameterxOES = f; return retval;}
    glPointParameterxvOES_t set_glPointParameterxvOES(glPointParameterxvOES_t f) { glPointParameterxvOES_t retval = glPointParameterxvOES; glPointParameterxvOES = f; return retval;}
    glPointSizexOES_t set_glPointSizexOES(glPointSizexOES_t f) { glPointSizexOES_t retval = glPointSizexOES; glPointSizexOES = f; return retval;}
    glPolygonOffsetxOES_t set_glPolygonOffsetxOES(glPolygonOffsetxOES_t f) { glPolygonOffsetxOES_t retval = glPolygonOffsetxOES; glPolygonOffsetxOES = f; return retval;}
    glRotatexOES_t set_glRotatexOES(glRotatexOES_t f) { glRotatexOES_t retval = glRotatexOES; glRotatexOES = f; return retval;}
    glSampleCoveragexOES_t set_glSampleCoveragexOES(glSampleCoveragexOES_t f) { glSampleCoveragexOES_t retval = glSampleCoveragexOES; glSampleCoveragexOES = f; return retval;}
    glScalexOES_t set_glScalexOES(glScalexOES_t f) { glScalexOES_t retval = glScalexOES; glScalexOES = f; return retval;}
    glTexEnvxOES_t set_glTexEnvxOES(glTexEnvxOES_t f) { glTexEnvxOES_t retval = glTexEnvxOES; glTexEnvxOES = f; return retval;}
    glTexEnvxvOES_t set_glTexEnvxvOES(glTexEnvxvOES_t f) { glTexEnvxvOES_t retval = glTexEnvxvOES; glTexEnvxvOES = f; return retval;}
    glTexParameterxOES_t set_glTexParameterxOES(glTexParameterxOES_t f) { glTexParameterxOES_t retval = glTexParameterxOES; glTexParameterxOES = f; return retval;}
    glTexParameterxvOES_t set_glTexParameterxvOES(glTexParameterxvOES_t f) { glTexParameterxvOES_t retval = glTexParameterxvOES; glTexParameterxvOES = f; return retval;}
    glTranslatexOES_t set_glTranslatexOES(glTranslatexOES_t f) { glTranslatexOES_t retval = glTranslatexOES; glTranslatexOES = f; return retval;}
    glIsRenderbufferOES_t set_glIsRenderbufferOES(glIsRenderbufferOES_t f) { glIsRenderbufferOES_t retval = glIsRenderbufferOES; glIsRenderbufferOES = f; return retval;}
    glBindRenderbufferOES_t set_glBindRenderbufferOES(glBindRenderbufferOES_t f) { glBindRenderbufferOES_t retval = glBindRenderbufferOES; glBindRenderbufferOES = f; return retval;}
    glDeleteRenderbuffersOES_t set_glDeleteRenderbuffersOES(glDeleteRenderbuffersOES_t f) { glDeleteRenderbuffersOES_t retval = glDeleteRenderbuffersOES; glDeleteRenderbuffersOES = f; return retval;}
    glGenRenderbuffersOES_t set_glGenRenderbuffersOES(glGenRenderbuffersOES_t f) { glGenRenderbuffersOES_t retval = glGenRenderbuffersOES; glGenRenderbuffersOES = f; return retval;}
    glRenderbufferStorageOES_t set_glRenderbufferStorageOES(glRenderbufferStorageOES_t f) { glRenderbufferStorageOES_t retval = glRenderbufferStorageOES; glRenderbufferStorageOES = f; return retval;}
    glGetRenderbufferParameterivOES_t set_glGetRenderbufferParameterivOES(glGetRenderbufferParameterivOES_t f) { glGetRenderbufferParameterivOES_t retval = glGetRenderbufferParameterivOES; glGetRenderbufferParameterivOES = f; return retval;}
    glIsFramebufferOES_t set_glIsFramebufferOES(glIsFramebufferOES_t f) { glIsFramebufferOES_t retval = glIsFramebufferOES; glIsFramebufferOES = f; return retval;}
    glBindFramebufferOES_t set_glBindFramebufferOES(glBindFramebufferOES_t f) { glBindFramebufferOES_t retval = glBindFramebufferOES; glBindFramebufferOES = f; return retval;}
    glDeleteFramebuffersOES_t set_glDeleteFramebuffersOES(glDeleteFramebuffersOES_t f) { glDeleteFramebuffersOES_t retval = glDeleteFramebuffersOES; glDeleteFramebuffersOES = f; return retval;}
    glGenFramebuffersOES_t set_glGenFramebuffersOES(glGenFramebuffersOES_t f) { glGenFramebuffersOES_t retval = glGenFramebuffersOES; glGenFramebuffersOES = f; return retval;}
    glCheckFramebufferStatusOES_t set_glCheckFramebufferStatusOES(glCheckFramebufferStatusOES_t f) { glCheckFramebufferStatusOES_t retval = glCheckFramebufferStatusOES; glCheckFramebufferStatusOES = f; return retval;}
    glFramebufferRenderbufferOES_t set_glFramebufferRenderbufferOES(glFramebufferRenderbufferOES_t f) { glFramebufferRenderbufferOES_t retval = glFramebufferRenderbufferOES; glFramebufferRenderbufferOES = f; return retval;}
    glFramebufferTexture2DOES_t set_glFramebufferTexture2DOES(glFramebufferTexture2DOES_t f) { glFramebufferTexture2DOES_t retval = glFramebufferTexture2DOES; glFramebufferTexture2DOES = f; return retval;}
    glGetFramebufferAttachmentParameterivOES_t set_glGetFramebufferAttachmentParameterivOES(glGetFramebufferAttachmentParameterivOES_t f) { glGetFramebufferAttachmentParameterivOES_t retval = glGetFramebufferAttachmentParameterivOES; glGetFramebufferAttachmentParameterivOES = f; return retval;}
    glGenerateMipmapOES_t set_glGenerateMipmapOES(glGenerateMipmapOES_t f) { glGenerateMipmapOES_t retval = glGenerateMipmapOES; glGenerateMipmapOES = f; return retval;}
    glMapBufferOES_t set_glMapBufferOES(glMapBufferOES_t f) { glMapBufferOES_t retval = glMapBufferOES; glMapBufferOES = f; return retval;}
    glUnmapBufferOES_t set_glUnmapBufferOES(glUnmapBufferOES_t f) { glUnmapBufferOES_t retval = glUnmapBufferOES; glUnmapBufferOES = f; return retval;}
    glGetBufferPointervOES_t set_glGetBufferPointervOES(glGetBufferPointervOES_t f) { glGetBufferPointervOES_t retval = glGetBufferPointervOES; glGetBufferPointervOES = f; return retval;}
    glCurrentPaletteMatrixOES_t set_glCurrentPaletteMatrixOES(glCurrentPaletteMatrixOES_t f) { glCurrentPaletteMatrixOES_t retval = glCurrentPaletteMatrixOES; glCurrentPaletteMatrixOES = f; return retval;}
    glLoadPaletteFromModelViewMatrixOES_t set_glLoadPaletteFromModelViewMatrixOES(glLoadPaletteFromModelViewMatrixOES_t f) { glLoadPaletteFromModelViewMatrixOES_t retval = glLoadPaletteFromModelViewMatrixOES; glLoadPaletteFromModelViewMatrixOES = f; return retval;}
    glMatrixIndexPointerOES_t set_glMatrixIndexPointerOES(glMatrixIndexPointerOES_t f) { glMatrixIndexPointerOES_t retval = glMatrixIndexPointerOES; glMatrixIndexPointerOES = f; return retval;}
    glWeightPointerOES_t set_glWeightPointerOES(glWeightPointerOES_t f) { glWeightPointerOES_t retval = glWeightPointerOES; glWeightPointerOES = f; return retval;}
    glQueryMatrixxOES_t set_glQueryMatrixxOES(glQueryMatrixxOES_t f) { glQueryMatrixxOES_t retval = glQueryMatrixxOES; glQueryMatrixxOES = f; return retval;}
    glDepthRangefOES_t set_glDepthRangefOES(glDepthRangefOES_t f) { glDepthRangefOES_t retval = glDepthRangefOES; glDepthRangefOES = f; return retval;}
    glFrustumfOES_t set_glFrustumfOES(glFrustumfOES_t f) { glFrustumfOES_t retval = glFrustumfOES; glFrustumfOES = f; return retval;}
    glOrthofOES_t set_glOrthofOES(glOrthofOES_t f) { glOrthofOES_t retval = glOrthofOES; glOrthofOES = f; return retval;}
    glClipPlanefOES_t set_glClipPlanefOES(glClipPlanefOES_t f) { glClipPlanefOES_t retval = glClipPlanefOES; glClipPlanefOES = f; return retval;}
    glGetClipPlanefOES_t set_glGetClipPlanefOES(glGetClipPlanefOES_t f) { glGetClipPlanefOES_t retval = glGetClipPlanefOES; glGetClipPlanefOES = f; return retval;}
    glClearDepthfOES_t set_glClearDepthfOES(glClearDepthfOES_t f) { glClearDepthfOES_t retval = glClearDepthfOES; glClearDepthfOES = f; return retval;}
    glTexGenfOES_t set_glTexGenfOES(glTexGenfOES_t f) { glTexGenfOES_t retval = glTexGenfOES; glTexGenfOES = f; return retval;}
    glTexGenfvOES_t set_glTexGenfvOES(glTexGenfvOES_t f) { glTexGenfvOES_t retval = glTexGenfvOES; glTexGenfvOES = f; return retval;}
    glTexGeniOES_t set_glTexGeniOES(glTexGeniOES_t f) { glTexGeniOES_t retval = glTexGeniOES; glTexGeniOES = f; return retval;}
    glTexGenivOES_t set_glTexGenivOES(glTexGenivOES_t f) { glTexGenivOES_t retval = glTexGenivOES; glTexGenivOES = f; return retval;}
    glTexGenxOES_t set_glTexGenxOES(glTexGenxOES_t f) { glTexGenxOES_t retval = glTexGenxOES; glTexGenxOES = f; return retval;}
    glTexGenxvOES_t set_glTexGenxvOES(glTexGenxvOES_t f) { glTexGenxvOES_t retval = glTexGenxvOES; glTexGenxvOES = f; return retval;}
    glGetTexGenfvOES_t set_glGetTexGenfvOES(glGetTexGenfvOES_t f) { glGetTexGenfvOES_t retval = glGetTexGenfvOES; glGetTexGenfvOES = f; return retval;}
    glGetTexGenivOES_t set_glGetTexGenivOES(glGetTexGenivOES_t f) { glGetTexGenivOES_t retval = glGetTexGenivOES; glGetTexGenivOES = f; return retval;}
    glGetTexGenxvOES_t set_glGetTexGenxvOES(glGetTexGenxvOES_t f) { glGetTexGenxvOES_t retval = glGetTexGenxvOES; glGetTexGenxvOES = f; return retval;}
    glBindVertexArrayOES_t set_glBindVertexArrayOES(glBindVertexArrayOES_t f) { glBindVertexArrayOES_t retval = glBindVertexArrayOES; glBindVertexArrayOES = f; return retval;}
    glDeleteVertexArraysOES_t set_glDeleteVertexArraysOES(glDeleteVertexArraysOES_t f) { glDeleteVertexArraysOES_t retval = glDeleteVertexArraysOES; glDeleteVertexArraysOES = f; return retval;}
    glGenVertexArraysOES_t set_glGenVertexArraysOES(glGenVertexArraysOES_t f) { glGenVertexArraysOES_t retval = glGenVertexArraysOES; glGenVertexArraysOES = f; return retval;}
    glIsVertexArrayOES_t set_glIsVertexArrayOES(glIsVertexArrayOES_t f) { glIsVertexArrayOES_t retval = glIsVertexArrayOES; glIsVertexArrayOES = f; return retval;}
    glDiscardFramebufferEXT_t set_glDiscardFramebufferEXT(glDiscardFramebufferEXT_t f) { glDiscardFramebufferEXT_t retval = glDiscardFramebufferEXT; glDiscardFramebufferEXT = f; return retval;}
    glMultiDrawArraysEXT_t set_glMultiDrawArraysEXT(glMultiDrawArraysEXT_t f) { glMultiDrawArraysEXT_t retval = glMultiDrawArraysEXT; glMultiDrawArraysEXT = f; return retval;}
    glMultiDrawElementsEXT_t set_glMultiDrawElementsEXT(glMultiDrawElementsEXT_t f) { glMultiDrawElementsEXT_t retval = glMultiDrawElementsEXT; glMultiDrawElementsEXT = f; return retval;}
    glClipPlanefIMG_t set_glClipPlanefIMG(glClipPlanefIMG_t f) { glClipPlanefIMG_t retval = glClipPlanefIMG; glClipPlanefIMG = f; return retval;}
    glClipPlanexIMG_t set_glClipPlanexIMG(glClipPlanexIMG_t f) { glClipPlanexIMG_t retval = glClipPlanexIMG; glClipPlanexIMG = f; return retval;}
    glRenderbufferStorageMultisampleIMG_t set_glRenderbufferStorageMultisampleIMG(glRenderbufferStorageMultisampleIMG_t f) { glRenderbufferStorageMultisampleIMG_t retval = glRenderbufferStorageMultisampleIMG; glRenderbufferStorageMultisampleIMG = f; return retval;}
    glFramebufferTexture2DMultisampleIMG_t set_glFramebufferTexture2DMultisampleIMG(glFramebufferTexture2DMultisampleIMG_t f) { glFramebufferTexture2DMultisampleIMG_t retval = glFramebufferTexture2DMultisampleIMG; glFramebufferTexture2DMultisampleIMG = f; return retval;}
    glDeleteFencesNV_t set_glDeleteFencesNV(glDeleteFencesNV_t f) { glDeleteFencesNV_t retval = glDeleteFencesNV; glDeleteFencesNV = f; return retval;}
    glGenFencesNV_t set_glGenFencesNV(glGenFencesNV_t f) { glGenFencesNV_t retval = glGenFencesNV; glGenFencesNV = f; return retval;}
    glIsFenceNV_t set_glIsFenceNV(glIsFenceNV_t f) { glIsFenceNV_t retval = glIsFenceNV; glIsFenceNV = f; return retval;}
    glTestFenceNV_t set_glTestFenceNV(glTestFenceNV_t f) { glTestFenceNV_t retval = glTestFenceNV; glTestFenceNV = f; return retval;}
    glGetFenceivNV_t set_glGetFenceivNV(glGetFenceivNV_t f) { glGetFenceivNV_t retval = glGetFenceivNV; glGetFenceivNV = f; return retval;}
    glFinishFenceNV_t set_glFinishFenceNV(glFinishFenceNV_t f) { glFinishFenceNV_t retval = glFinishFenceNV; glFinishFenceNV = f; return retval;}
    glSetFenceNV_t set_glSetFenceNV(glSetFenceNV_t f) { glSetFenceNV_t retval = glSetFenceNV; glSetFenceNV = f; return retval;}
    glGetDriverControlsQCOM_t set_glGetDriverControlsQCOM(glGetDriverControlsQCOM_t f) { glGetDriverControlsQCOM_t retval = glGetDriverControlsQCOM; glGetDriverControlsQCOM = f; return retval;}
    glGetDriverControlStringQCOM_t set_glGetDriverControlStringQCOM(glGetDriverControlStringQCOM_t f) { glGetDriverControlStringQCOM_t retval = glGetDriverControlStringQCOM; glGetDriverControlStringQCOM = f; return retval;}
    glEnableDriverControlQCOM_t set_glEnableDriverControlQCOM(glEnableDriverControlQCOM_t f) { glEnableDriverControlQCOM_t retval = glEnableDriverControlQCOM; glEnableDriverControlQCOM = f; return retval;}
    glDisableDriverControlQCOM_t set_glDisableDriverControlQCOM(glDisableDriverControlQCOM_t f) { glDisableDriverControlQCOM_t retval = glDisableDriverControlQCOM; glDisableDriverControlQCOM = f; return retval;}
    glExtGetTexturesQCOM_t set_glExtGetTexturesQCOM(glExtGetTexturesQCOM_t f) { glExtGetTexturesQCOM_t retval = glExtGetTexturesQCOM; glExtGetTexturesQCOM = f; return retval;}
    glExtGetBuffersQCOM_t set_glExtGetBuffersQCOM(glExtGetBuffersQCOM_t f) { glExtGetBuffersQCOM_t retval = glExtGetBuffersQCOM; glExtGetBuffersQCOM = f; return retval;}
    glExtGetRenderbuffersQCOM_t set_glExtGetRenderbuffersQCOM(glExtGetRenderbuffersQCOM_t f) { glExtGetRenderbuffersQCOM_t retval = glExtGetRenderbuffersQCOM; glExtGetRenderbuffersQCOM = f; return retval;}
    glExtGetFramebuffersQCOM_t set_glExtGetFramebuffersQCOM(glExtGetFramebuffersQCOM_t f) { glExtGetFramebuffersQCOM_t retval = glExtGetFramebuffersQCOM; glExtGetFramebuffersQCOM = f; return retval;}
    glExtGetTexLevelParameterivQCOM_t set_glExtGetTexLevelParameterivQCOM(glExtGetTexLevelParameterivQCOM_t f) { glExtGetTexLevelParameterivQCOM_t retval = glExtGetTexLevelParameterivQCOM; glExtGetTexLevelParameterivQCOM = f; return retval;}
    glExtTexObjectStateOverrideiQCOM_t set_glExtTexObjectStateOverrideiQCOM(glExtTexObjectStateOverrideiQCOM_t f) { glExtTexObjectStateOverrideiQCOM_t retval = glExtTexObjectStateOverrideiQCOM; glExtTexObjectStateOverrideiQCOM = f; return retval;}
    glExtGetTexSubImageQCOM_t set_glExtGetTexSubImageQCOM(glExtGetTexSubImageQCOM_t f) { glExtGetTexSubImageQCOM_t retval = glExtGetTexSubImageQCOM; glExtGetTexSubImageQCOM = f; return retval;}
    glExtGetBufferPointervQCOM_t set_glExtGetBufferPointervQCOM(glExtGetBufferPointervQCOM_t f) { glExtGetBufferPointervQCOM_t retval = glExtGetBufferPointervQCOM; glExtGetBufferPointervQCOM = f; return retval;}
    glExtGetShadersQCOM_t set_glExtGetShadersQCOM(glExtGetShadersQCOM_t f) { glExtGetShadersQCOM_t retval = glExtGetShadersQCOM; glExtGetShadersQCOM = f; return retval;}
    glExtGetProgramsQCOM_t set_glExtGetProgramsQCOM(glExtGetProgramsQCOM_t f) { glExtGetProgramsQCOM_t retval = glExtGetProgramsQCOM; glExtGetProgramsQCOM = f; return retval;}
    glExtIsProgramBinaryQCOM_t set_glExtIsProgramBinaryQCOM(glExtIsProgramBinaryQCOM_t f) { glExtIsProgramBinaryQCOM_t retval = glExtIsProgramBinaryQCOM; glExtIsProgramBinaryQCOM = f; return retval;}
    glExtGetProgramBinarySourceQCOM_t set_glExtGetProgramBinarySourceQCOM(glExtGetProgramBinarySourceQCOM_t f) { glExtGetProgramBinarySourceQCOM_t retval = glExtGetProgramBinarySourceQCOM; glExtGetProgramBinarySourceQCOM = f; return retval;}
    glStartTilingQCOM_t set_glStartTilingQCOM(glStartTilingQCOM_t f) { glStartTilingQCOM_t retval = glStartTilingQCOM; glStartTilingQCOM = f; return retval;}
    glEndTilingQCOM_t set_glEndTilingQCOM(glEndTilingQCOM_t f) { glEndTilingQCOM_t retval = glEndTilingQCOM; glEndTilingQCOM = f; return retval;}
};

gles_dispatch *create_gles_dispatch(void *gles_andorid);

#endif
