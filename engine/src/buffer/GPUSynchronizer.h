#pragma once

#include "sausage.h"

using namespace std;

class GPUSynchronizer {
  GLsync fence_sync = 0;
  bool is_need_barrier = false;

public:
  static GPUSynchronizer* GetInstance() {
    static GPUSynchronizer* gpu_synchronizer = new GPUSynchronizer();
    return gpu_synchronizer;
  }
  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  void SetSyncBarrier() {
    is_need_barrier = true;
  }
  void SyncGPU() {
    WaitGPU(fence_sync);
    if (is_need_barrier) {
      glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
      is_need_barrier = false;
    }
  }
  void PostDraw() {
    fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    DEBUG_EXPR(CheckGLError());
  }
  void WaitGPU(GLsync fence_sync,
               const source_location& location = source_location::current()) {
    if (fence_sync == 0) {
      return;
    }
    GLbitfield waitFlags = 0;
    GLuint64 waitDuration = 0;
    while (true) {
      GLenum waitRet = glClientWaitSync(fence_sync, waitFlags, waitDuration);
      if (waitRet ==  GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
        return;
      }

      if (waitRet == GL_WAIT_FAILED) {
        LOG((
          ostringstream()
          << "WaitGPU wait sync returned status GL_WAIT_FAILED at"
          << location.file_name() << "(" << location.line() << ":"
          << location.column() << ")#" << location.function_name())
          .str());
        return;
      }

      // After the first time, need to start flushing, and wait for a looong time.
      waitFlags = GL_SYNC_FLUSH_COMMANDS_BIT;
      waitDuration = 1000000000;  // one second in nanoseconds
    }
    glDeleteSync(fence_sync);
    fence_sync = 0;
  }
private:
  GPUSynchronizer() {};
};
