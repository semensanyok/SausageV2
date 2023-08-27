/*
Decided to store all commands in memory and pass as* indirect parameter to
```
  void glMultiDrawElementsIndirect(GLenum mode,
    GLenum type,
    const void* indirect,
    GLsizei drawcount,
    GLsizei stride);
```

Because frustum cull gathers new contigious array of commands each frame
and mapping/unmapping GPU buffer + managing buffer slots via `Arena` each frame is cumbersome and costly?

NOTES:
- command array parameter on RAM is deprecated.
OK for core or compatibility profiles:
```
  glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT,
   nullptr,
   command_count, 0);
```

OK only for compatibility profile
```
  glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT,
   draw->commands.data(),
   //nullptr,
   command_count, 0);
```
*/

#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "GPUSynchronizer.h"
#include "Vertex.h"
#include "OpenGLHelpers.h"
#include "BufferSettings.h"
#include "GLBuffersCommon.h"
#include "GPUSynchronizer.h";

using namespace std;

struct BufferLock {
  mutex data_mutex;
  condition_variable is_mapped_cv;
  bool is_mapped;
  inline void Wait(unique_lock<mutex>& data_lock) {
    is_mapped_cv.wait(data_lock);
  }
};

struct CommandBuffer {
  BufferSlots<DrawElementsIndirectCommand>* ptr;
  // we have to use lock here
  // because command buffer must be "unmapped" before each drawcall
  BufferLock* buffer_lock;
  inline bool operator==(const CommandBuffer& other) { return ptr->buffer_id == other.ptr->buffer_id; }
  ~CommandBuffer() {
    delete buffer_lock;
  }
};

struct CommandBuffers {
  CommandBuffer* blinn_phong;
  //CommandBuffer* stencil;
  CommandBuffer* font_ui;
  CommandBuffer* back_ui;
  //CommandBuffer* overlay_3d;
  CommandBuffer* outline;
  CommandBuffer* mesh_static;
  CommandBuffer* terrain;
};

class CommandBuffersManager {
  vector<CommandBuffer*> mapped_command_buffers;
public:
  static CommandBuffersManager* GetInstance() {
    static CommandBuffersManager* instance = new CommandBuffersManager();
    return instance;
  }

  CommandBuffers command_buffers;

  void InitBuffers() {
    command_buffers.font_ui = CreateCommandBuffer(GetNumDrawCommandsForFontDrawCall());
    command_buffers.back_ui = CreateCommandBuffer(GetNumDrawCommandsForBackDrawCall());
    command_buffers.outline = CreateCommandBuffer(1);
    command_buffers.blinn_phong = CreateCommandBuffer(MAX_BASE_MESHES);
    command_buffers.mesh_static = CreateCommandBuffer(MAX_BASE_MESHES_STATIC);
    command_buffers.terrain = CreateCommandBuffer(MAX_MESHES_TERRAIN);
  }

  void Reset() {
    command_buffers.blinn_phong->ptr->Reset();
    command_buffers.font_ui->ptr->Reset();
    command_buffers.back_ui->ptr->Reset();
    command_buffers.outline->ptr->Reset();
    command_buffers.mesh_static->ptr->Reset();
    command_buffers.terrain->ptr->Reset();
  }

  void BufferCommands(
    CommandBuffer* command_ptr,
    vector<DrawElementsIndirectCommand>& commands,
    int command_offset) {
    unique_lock<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
    if (!command_ptr->buffer_lock->is_mapped) {
      command_ptr->buffer_lock->Wait(data_lock);
    }
    memcpy(&(command_ptr->ptr->buffer_ptr[command_offset]),
           commands.data(),
           commands.size() * sizeof(DrawElementsIndirectCommand));
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  }
  void BufferCommand(CommandBuffer* command_ptr,
    DrawElementsIndirectCommand& command, int command_offset) {
    unique_lock<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
    while (!command_ptr->buffer_lock->is_mapped) {
      command_ptr->buffer_lock->Wait(data_lock);
    }
    command_ptr->ptr->buffer_ptr[command_offset] = command;
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  }

  //void PreDraw() {
  //  UnmapBuffers();
  //}

  //void PostDraw() {
  //  MapBuffers();
  //}


  // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
  void UnmapBuffer(CommandBuffer* buf) {
    lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
    if (buf->buffer_lock->is_mapped == false) {
      return;
    }
    buf->buffer_lock->is_mapped = false;
    buf->buffer_lock->is_mapped_cv.notify_all();
    if (!glUnmapNamedBuffer(buf->ptr->buffer_id)) {
      DEBUG_EXPR(CheckGLError());
    }
  }

  void MapBuffer(CommandBuffer* buf) {
    lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
    if (buf->buffer_lock->is_mapped == true) {
      return;
    }
    auto sz = buf->ptr->instances_slots.instances_slots.GetSize();
    buf->ptr->buffer_ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(
      buf->ptr->buffer_id,
      0,
      sz * sizeof(DrawElementsIndirectCommand),
      GLBuffersCommon::flags);
    DEBUG_EXPR(CheckGLError());
    buf->buffer_lock->is_mapped = true;
    buf->buffer_lock->is_mapped_cv.notify_all();

  }

private:
  CommandBuffer* CreateCommandBuffer(unsigned int num_commands) {
    auto size = sizeof(DrawElementsIndirectCommand) * num_commands;
    CommandBuffer* command_ptr = new CommandBuffer{};
    command_ptr->ptr = CreateBufferSlots<DrawElementsIndirectCommand>(size,
      num_commands,
      GL_DRAW_INDIRECT_BUFFER,
      //ArenaSlotSize::FOUR
      ArenaSlotSize::ONE
      );
    command_ptr->buffer_lock = new BufferLock();
    command_ptr->buffer_lock->is_mapped = true;
    DEBUG_EXPR(CheckGLError());

    return command_ptr;
  }

  //void UnmapBuffers() {
  //  UnmapBuffer(command_buffers.blinn_phong);
  //  UnmapBuffer(command_buffers.mesh_static);
  //  UnmapBuffer(command_buffers.terrain);
  //  UnmapBuffer(command_buffers.back_ui);
  //  UnmapBuffer(command_buffers.font_ui);
  //  UnmapBuffer(command_buffers.outline);
  //}

  //void MapBuffers() {
  //  MapBuffer(command_buffers.blinn_phong);
  //  MapBuffer(command_buffers.mesh_static);
  //  MapBuffer(command_buffers.terrain);
  //  MapBuffer(command_buffers.font_ui);
  //  MapBuffer(command_buffers.back_ui);
  //  MapBuffer(command_buffers.outline);
  //}

  //void MapBuffersMesh() {
  //  MapBuffer(command_buffers.blinn_phong);
  //  MapBuffer(command_buffers.outline);
  //}
  //void MapBuffersTerrain() {
  //  MapBuffer(command_buffers.terrain);
  //}
  //void MapBuffersUI() {
  //  MapBuffer(command_buffers.back_ui);
  //  MapBuffer(command_buffers.font_ui);
  //}

  void Dispose() {
    GPUSynchronizer::GetInstance()->SyncGPU();

    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.font_ui->ptr->buffer_id);
    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.back_ui->ptr->buffer_id);
    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.blinn_phong->ptr->buffer_id);
    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.outline->ptr->buffer_id);
    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.mesh_static->ptr->buffer_id);
    DeleteBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffers.terrain->ptr->buffer_id);
  }
  private:
    CommandBuffersManager() {};
};
