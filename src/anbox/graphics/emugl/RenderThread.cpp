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

#include "anbox/graphics/emugl/RenderThread.h"
#include "anbox/graphics/emugl/ReadBuffer.h"
#include "anbox/graphics/emugl/RenderControl.h"
#include "anbox/graphics/emugl/RenderThreadInfo.h"
#include "anbox/graphics/emugl/Renderer.h"
#include "anbox/graphics/emugl/TimeUtils.h"
#include "anbox/logger.h"

#include "external/android-emugl/shared/OpenglCodecCommon/ChecksumCalculatorThreadInfo.h"
#include "external/android-emugl/host/include/OpenGLESDispatch/EGLDispatch.h"
#include "external/android-emugl/host/include/OpenGLESDispatch/GLESv1Dispatch.h"
#include "external/android-emugl/host/include/OpenGLESDispatch/GLESv2Dispatch.h"

#define STREAM_BUFFER_SIZE 4 * 1024 * 1024

RenderThread::RenderThread(const std::shared_ptr<Renderer> &renderer, IOStream *stream, std::mutex &m)
    : emugl::Thread(), renderer_(renderer), m_lock(m), m_stream(stream) {}

RenderThread::~RenderThread() {
  forceStop();
}

RenderThread *RenderThread::create(const std::shared_ptr<Renderer> &renderer, IOStream *stream, std::mutex &m) {
  return new RenderThread(renderer, stream, m);
}

void RenderThread::forceStop() { m_stream->forceStop(); }

intptr_t RenderThread::main() {
  RenderThreadInfo threadInfo;
  ChecksumCalculatorThreadInfo threadChecksumInfo;

  threadInfo.m_glDec.initGL(gles1_dispatch_get_proc_func, NULL);
  threadInfo.m_gl2Dec.initGL(gles2_dispatch_get_proc_func, NULL);
  initRenderControlContext(&threadInfo.m_rcDec);

  ReadBuffer readBuf(STREAM_BUFFER_SIZE);

  while (true) {
    int stat = readBuf.getData(m_stream);
    if (stat <= 0)
      break;

    bool progress;
    do {
      progress = false;

      std::unique_lock<std::mutex> l(m_lock);

      size_t last =
          threadInfo.m_glDec.decode(readBuf.buf(), readBuf.validData(), m_stream);
      if (last > 0) {
        progress = true;
        readBuf.consume(last);
      }

      last =
          threadInfo.m_gl2Dec.decode(readBuf.buf(), readBuf.validData(), m_stream);
      if (last > 0) {
        progress = true;
        readBuf.consume(last);
      }

      last = threadInfo.m_rcDec.decode(readBuf.buf(), readBuf.validData(), m_stream);
      if (last > 0) {
        readBuf.consume(last);
        progress = true;
      }

    } while (progress);

  }

  threadInfo.m_gl2Dec.freeShader();
  threadInfo.m_gl2Dec.freeProgram();

  // Release references to the current thread's context/surfaces if any
  renderer_->bindContext(0, 0, 0);
  if (threadInfo.currContext || threadInfo.currDrawSurf || threadInfo.currReadSurf)
    ERROR("RenderThread exiting with current context/surfaces");

  renderer_->drainWindowSurface();
  renderer_->drainRenderContext();

  return 0;
}
