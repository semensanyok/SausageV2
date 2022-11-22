#include "GLBuffers.h"
#include "BufferStorage.h"

void GLBuffers::AddUsedBuffers(BufferType::BufferTypeFlag used_buffers) {
  this->used_buffers |= used_buffers;
}
void GLBuffers::MapBuffers() {
  MapBuffer(command_buffers.blinn_phong);
  MapBuffer(command_buffers.back_ui);
  MapBuffer(command_buffers.font_ui);
  MapBuffer(command_buffers.bullet_debug);
}
void GLBuffers::MapBuffer(CommandBuffer* buf) {
  lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
  if (buf->buffer_lock->is_mapped == true) {
    return;
  }
  // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
  buf->ptr->buffer_ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(
    buf->ptr->buffer_id, 0, buf->ptr->instances_slots.instances_slots.GetSize(), flags);
  buf->buffer_lock->is_mapped = true;
  buf->buffer_lock->is_mapped_cv.notify_all();
}

void GLBuffers::PreDraw() {
  _SyncGPUBufAndUnmap();
  BindVAOandBuffers();
  DEBUG_EXPR(CheckGLError());
}

void GLBuffers::PostDraw() {
  fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  MapBuffers();
  DEBUG_EXPR(CheckGLError());
}

void GLBuffers::UnmapBuffers() {
  UnmapBuffer(command_buffers.blinn_phong);
  UnmapBuffer(command_buffers.back_ui);
  UnmapBuffer(command_buffers.font_ui);
  UnmapBuffer(command_buffers.bullet_debug);
}

void GLBuffers::UnmapBuffer(CommandBuffer* buf) {
  lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
  if (buf->buffer_lock->is_mapped == false) {
    return;
  }
  buf->buffer_lock->is_mapped = false;
  buf->buffer_lock->is_mapped_cv.notify_all();
  // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
  if (!glUnmapNamedBuffer(buf->ptr->buffer_id)) {
    DEBUG_EXPR(CheckGLError());
  }
}

void GLBuffers::_SyncGPUBufAndUnmap() {
  WaitGPU(fence_sync);
  if (is_need_barrier) {
    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    is_need_barrier = false;
  }
  UnmapBuffers();
}

void GLBuffers::WaitGPU(GLsync fence_sync, const source_location& location) {
  if (fence_sync == 0) {
    return;
  }
  GLbitfield waitFlags = 0;
  GLuint64 waitDuration = 0;
  while (true) {
    GLenum waitRet = glClientWaitSync(fence_sync, waitFlags, waitDuration);
    if (waitRet == GL_ALREADY_SIGNALED || waitRet == GL_CONDITION_SATISFIED) {
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

void GLBuffers::InitBuffers() {
  glGenVertexArrays(1, &mesh_VAO);
  glBindVertexArray(
    mesh_VAO);  // MUST be bound before glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
                // command_ptr).

  vertex_ptr = _CreateBufferStorageSlots<Vertex>(VERTEX_STORAGE_SIZE, GL_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
  index_ptr = _CreateBufferStorageSlots<unsigned int>(INDEX_STORAGE_SIZE, GL_ELEMENT_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
  mesh_uniform_ptr = _CreateBufferStorageSlots<UniformDataMesh>(MESH_UNIFORMS_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  texture_handle_by_texture_id_ptr = _CreateBufferStorageNumberPool<GLuint64>(TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  light_ptr = _CreateBufferStorageSlots<LightsUniform>(LIGHT_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_3d_overlay_ptr = _CreateBufferStorageSlots<UniformDataOverlay3D>(UNIFORM_OVERLAY_3D_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_ui_ptr = _CreateBufferStorageSlots<UniformDataUI>(UNIFORM_UI_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_controller_ptr = _CreateBufferStorageSlots<ControllerUniformData>(UNIFORM_CONTROLLER_SIZE, GL_SHADER_STORAGE_BUFFER);

  command_buffers = CommandBuffers();
  command_buffers.font_ui = CreateCommandBuffer(GetNumDrawCommandsForFontDrawCall());
  command_buffers.back_ui = CreateCommandBuffer(GetNumDrawCommandsForBackDrawCall());
  command_buffers.bullet_debug = CreateCommandBuffer(1);
  command_buffers.blinn_phong = CreateCommandBuffer(MAX_BASE_MESHES);
}

CommandBuffer* GLBuffers::CreateCommandBuffer(unsigned int num_commands) {
  auto size = num_commands;
  CommandBuffer* command_ptr = new CommandBuffer{};
  command_ptr->ptr = _CreateBufferStorageSlots<DrawElementsIndirectCommand>(size,
    GL_SHADER_STORAGE_BUFFER,
    //ArenaSlotSize::FOUR
    ArenaSlotSize::ONE
    );
  command_ptr->buffer_lock = new BufferLock();
  command_ptr->buffer_lock->is_mapped = true;
  DEBUG_EXPR(CheckGLError());

  return command_ptr;
}

template<typename T>
BufferSlots<T>* GLBuffers::_CreateBufferStorageSlots(unsigned long storage_size,
  GLuint array_type,
  ArenaSlotSize slot_size) {
  GLuint buffer_id;
  glGenBuffers(1, &buffer_id);
  glBindBuffer(array_type, buffer_id);
  glBufferStorage(array_type, storage_size, NULL, flags);
  T* buffer_ptr = (T*)glMapBufferRange(array_type, 0,
    storage_size, flags);
  return new BufferSlots{ {Arena({ 0, storage_size }, slot_size)}, buffer_id , buffer_ptr };
}

template<typename T>
BufferNumberPool<T>* GLBuffers::_CreateBufferStorageNumberPool(unsigned long storage_size,
    GLuint array_type) {
  GLuint buffer_id;
  glGenBuffers(1, &buffer_id);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer_id);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, storage_size, NULL, flags);
  T* buffer_ptr = (T*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
    storage_size, flags);
  return new BufferNumberPool{ ThreadSafeNumberPool(storage_size), buffer_id , buffer_ptr };
}

void GLBuffers::BindVAOandBuffers() {
  // TODO: separate Font VAO. it needs less data.
  if ((used_buffers & BufferType::MESH_VAO) &&
    !(bound_buffers & BufferType::MESH_VAO)) {
    glBindVertexArray(mesh_VAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
      (void*)offsetof(Vertex, BoneIds));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, BoneWeights));
    bound_buffers |= BufferType::MESH_VAO;
  }
  if ((used_buffers & BufferType::VERTEX) &&
    !(bound_buffers & BufferType::VERTEX)) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_ptr->buffer_id);
    bound_buffers |= BufferType::VERTEX;
  }
  if ((used_buffers & BufferType::INDEX) &&
    !(bound_buffers & BufferType::INDEX)) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ptr->buffer_id);
    bound_buffers |= BufferType::INDEX;
  }
  if ((used_buffers & BufferType::UNIFORMS) &&
    !(bound_buffers & BufferType::UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_uniform_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UNIFORMS_LOC, mesh_uniform_ptr->buffer_id);
    bound_buffers |= BufferType::UNIFORMS;
  }

  if ((used_buffers & BufferType::TEXTURE) &&
    !(bound_buffers & BufferType::TEXTURE)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_LOC,
      texture_handle_by_texture_id_ptr->buffer_id);
    bound_buffers |= BufferType::TEXTURE;
  }
  if ((used_buffers & BufferType::LIGHT) &&
    !(bound_buffers & BufferType::LIGHT)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_UNIFORM_LOC,
      light_ptr->buffer_id);
    bound_buffers |= BufferType::LIGHT;
  }
  if ((used_buffers & BufferType::UI_UNIFORMS) &&
    !(bound_buffers & BufferType::UI_UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, FONT_UNIFORMS_LOC,
      uniforms_3d_overlay_ptr->buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UI_UNIFORM_LOC,
      uniforms_ui_ptr->buffer_id);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_controller_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CONTROLLER_UNIFORM_LOC,
      uniforms_controller_ptr->buffer_id);
    bound_buffers |= BufferType::UI_UNIFORMS;
  }
}

void GLBuffers::Dispose() {
  _SyncGPUBufAndUnmap();

  glDisableVertexAttribArray(0);
  glDeleteVertexArrays(1, &mesh_VAO);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_ARRAY_BUFFER, vertex_ptr->buffer_id);
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glDeleteBuffers(1, &vertex_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ptr->buffer_id);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glDeleteBuffers(1, &index_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_uniform_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &mesh_uniform_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &texture_handle_by_texture_id_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &light_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  _DeleteCommandBuffer(command_buffers.font_ui);
  _DeleteCommandBuffer(command_buffers.back_ui);
  _DeleteCommandBuffer(command_buffers.blinn_phong);
  _DeleteCommandBuffer(command_buffers.bullet_debug);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_ui_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_3d_overlay_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_controller_ptr->buffer_id);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_controller_ptr->buffer_id);
  DEBUG_EXPR(CheckGLError());
}
