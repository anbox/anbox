#ifndef GLES_MACROS_H
#define GLES_MACROS_H

#define GET_CTX() \
            if(!s_eglIface) return; \
            GLEScontext *ctx = s_eglIface->getGLESContext(); \
            if(!ctx) return;

#define GET_CTX_CM() \
            if(!s_eglIface) return; \
            GLEScmContext *ctx = static_cast<GLEScmContext *>(s_eglIface->getGLESContext()); \
            if(!ctx) return;

#define GET_CTX_V2() \
            if(!s_eglIface) return; \
            GLESv2Context *ctx = static_cast<GLESv2Context *>(s_eglIface->getGLESContext()); \
            if(!ctx) return;

#define GET_CTX_RET(failure_ret) \
            if(!s_eglIface) return failure_ret; \
            GLEScontext *ctx = s_eglIface->getGLESContext(); \
            if(!ctx) return failure_ret;

#define GET_CTX_CM_RET(failure_ret) \
            if(!s_eglIface) return failure_ret; \
            GLEScmContext *ctx = static_cast<GLEScmContext *>(s_eglIface->getGLESContext()); \
            if(!ctx) return failure_ret;

#define GET_CTX_V2_RET(failure_ret) \
            if(!s_eglIface) return failure_ret; \
            GLESv2Context *ctx = static_cast<GLESv2Context *>(s_eglIface->getGLESContext()); \
            if(!ctx) return failure_ret;


#define SET_ERROR_IF(condition,err) if((condition)) {                            \
                        fprintf(stderr, "%s:%s:%d error 0x%x\n", __FILE__, __FUNCTION__, __LINE__, err); \
                        ctx->setGLerror(err);                                    \
                        return;                                                  \
                    }


#define RET_AND_SET_ERROR_IF(condition,err,ret) if((condition)) {                \
                        fprintf(stderr, "%s:%s:%d error 0x%x\n", __FILE__, __FUNCTION__, __LINE__, err); \
                        ctx->setGLerror(err);                                    \
                        return ret;                                              \
                    }

#endif
