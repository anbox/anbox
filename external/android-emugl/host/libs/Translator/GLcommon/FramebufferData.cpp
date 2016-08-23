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
#include <GLES/gl.h>
#include <GLES/glext.h>
#include <GLcommon/FramebufferData.h>
#include <GLcommon/GLEScontext.h>

RenderbufferData::RenderbufferData() : sourceEGLImage(0),
                         eglImageDetach(NULL),
                         attachedFB(0),
                         attachedPoint(0),
                         eglImageGlobalTexName(0) {
}

RenderbufferData::~RenderbufferData() {
    if (sourceEGLImage && eglImageDetach) (*eglImageDetach)(sourceEGLImage);
}


FramebufferData::FramebufferData(GLuint name):m_dirty(false) {
    m_fbName = name;
    for (int i=0; i<MAX_ATTACH_POINTS; i++) {
        m_attachPoints[i].target = 0;
        m_attachPoints[i].name = 0;
        m_attachPoints[i].obj = ObjectDataPtr(NULL);
        m_attachPoints[i].owned = false;
    }
}

FramebufferData::~FramebufferData() {
for (int i=0; i<MAX_ATTACH_POINTS; i++) {
    detachObject(i);
}
}

void FramebufferData::setAttachment(GLenum attachment,
               GLenum target,
               GLuint name,
               ObjectDataPtr obj,
               bool takeOwnership) {
int idx = attachmentPointIndex(attachment);
    if (!name) {
        detachObject(idx);
        return;
    }
    if (m_attachPoints[idx].target != target ||
        m_attachPoints[idx].name != name ||
        m_attachPoints[idx].obj.Ptr() != obj.Ptr() ||
        m_attachPoints[idx].owned != takeOwnership) {

        detachObject(idx); 

        m_attachPoints[idx].target = target;
        m_attachPoints[idx].name = name;
        m_attachPoints[idx].obj = obj;
        m_attachPoints[idx].owned = takeOwnership;

        if (target == GL_RENDERBUFFER_OES && obj.Ptr() != NULL) {
            RenderbufferData *rbData = (RenderbufferData *)obj.Ptr();
            rbData->attachedFB = m_fbName;
            rbData->attachedPoint = attachment;
        }

        m_dirty = true;
    }
}

GLuint FramebufferData::getAttachment(GLenum attachment,
                 GLenum *outTarget,
                 ObjectDataPtr *outObj) {
    int idx = attachmentPointIndex(attachment);
    if (outTarget) *outTarget = m_attachPoints[idx].target;
    if (outObj) *outObj = m_attachPoints[idx].obj;
    return m_attachPoints[idx].name;
}

int FramebufferData::attachmentPointIndex(GLenum attachment)
{
    switch(attachment) {
    case GL_COLOR_ATTACHMENT0_OES:
        return 0;
    case GL_DEPTH_ATTACHMENT_OES:
        return 1;
    case GL_STENCIL_ATTACHMENT_OES:
        return 2;
    default:
        return MAX_ATTACH_POINTS;
    }
}

void FramebufferData::detachObject(int idx) {
    if (m_attachPoints[idx].target == GL_RENDERBUFFER_OES && m_attachPoints[idx].obj.Ptr() != NULL) {
        RenderbufferData *rbData = (RenderbufferData *)m_attachPoints[idx].obj.Ptr();
        rbData->attachedFB = 0;
        rbData->attachedPoint = 0;
    }

    if(m_attachPoints[idx].owned)
    {
        switch(m_attachPoints[idx].target)
        {
        case GL_RENDERBUFFER_OES:
            GLEScontext::dispatcher().glDeleteRenderbuffersEXT(1, &(m_attachPoints[idx].name));
            break;
        case GL_TEXTURE_2D:
            GLEScontext::dispatcher().glDeleteTextures(1, &(m_attachPoints[idx].name));
            break;
        }
    }

    m_attachPoints[idx].target = 0;
    m_attachPoints[idx].name = 0;
    m_attachPoints[idx].obj = ObjectDataPtr(NULL);
    m_attachPoints[idx].owned = false;
}

void FramebufferData::validate(GLEScontext* ctx)
{
    if(!getAttachment(GL_COLOR_ATTACHMENT0_OES, NULL, NULL))
    {
        // GLES does not require the framebuffer to have a color attachment.
        // OpenGL does. Therefore, if no color is attached, create a dummy
        // color texture and attach it.
        // This dummy color texture will is owned by the FramebufferObject,
        // and will be released by it when its object is detached.

        GLint type = GL_NONE;
        GLint name = 0;

        ctx->dispatcher().glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
        if(type != GL_NONE)
        {
            ctx->dispatcher().glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
        }
        else
        {
            ctx->dispatcher().glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE, &type);
            if(type != GL_NONE)
            {
                ctx->dispatcher().glGetFramebufferAttachmentParameterivEXT(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT_OES, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME, &name);
            }
            else
            {
                // No color, depth or stencil attachments - do nothing
                return;
            }
        }

        // Find the existing attachment(s) dimensions
        GLint width = 0;
        GLint height = 0;

        if(type == GL_RENDERBUFFER)
        {
            GLint prev;
            ctx->dispatcher().glGetIntegerv(GL_RENDERBUFFER_BINDING, &prev);
            ctx->dispatcher().glBindRenderbufferEXT(GL_RENDERBUFFER, name);
            ctx->dispatcher().glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &width);
            ctx->dispatcher().glGetRenderbufferParameterivEXT(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &height);
            ctx->dispatcher().glBindRenderbufferEXT(GL_RENDERBUFFER, prev);
        }
        else if(type == GL_TEXTURE)
        {
            GLint prev;
            ctx->dispatcher().glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev);
            ctx->dispatcher().glBindTexture(GL_TEXTURE_2D, name);
            ctx->dispatcher().glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &width);
            ctx->dispatcher().glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &height);
            ctx->dispatcher().glBindTexture(GL_TEXTURE_2D, prev);
        }

        // Create the color attachment and attch it
        unsigned int tex = ctx->shareGroup()->genGlobalName(TEXTURE);
        GLint prev;
        ctx->dispatcher().glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev);
        ctx->dispatcher().glBindTexture(GL_TEXTURE_2D, tex);

        ctx->dispatcher().glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        ctx->dispatcher().glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
        ctx->dispatcher().glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
        ctx->dispatcher().glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
        ctx->dispatcher().glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        ctx->dispatcher().glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, tex, 0);
        setAttachment(GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, tex, ObjectDataPtr(NULL), true);

        ctx->dispatcher().glBindTexture(GL_TEXTURE_2D, prev);
    }

    if(m_dirty)
    {
        // This is a workaround for a bug found in several OpenGL
        // drivers (e.g. ATI's) - after the framebuffer attachments
        // have changed, and before the next draw, unbind and rebind
        // the framebuffer to sort things out.
        ctx->dispatcher().glBindFramebufferEXT(GL_FRAMEBUFFER,0);
        ctx->dispatcher().glBindFramebufferEXT(GL_FRAMEBUFFER,ctx->shareGroup()->getGlobalName(FRAMEBUFFER,m_fbName));

        m_dirty = false;
    }
}

