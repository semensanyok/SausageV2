#include "GLBuffers.h"


void GLBuffers::AddUsedBuffers(BufferType::BufferTypeFlag used_buffers) {
  this->used_buffers |= used_buffers;
}

void GLBuffers::MapBuffer() {
  lock_guard<mutex> data_lock(command_buffer->buffer_lock->data_mutex);
  if (command_buffer->buffer_lock->is_mapped == true) {
    return;
  }
  // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
  command_buffer->ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(
    command_buffer->id, 0, COMMAND_STORAGE_SIZE, flags);
  command_buffer->buffer_lock->is_mapped = true;
  command_buffer->buffer_lock->is_mapped_cv.notify_all();
}

void GLBuffers::PreDraw() {
  _SyncGPUBufAndUnmap();
  BindVAOandBuffers(); // TODO: one buffer, no rebind
}

void GLBuffers::PostDraw() {
  fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
  MapBuffer();
}

void GLBuffers::_BindCommandBuffer() {
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer->id);
}

void GLBuffers::_UnmapBuffer() {
  lock_guard<mutex> data_lock(command_buffer->buffer_lock->data_mutex);
  if (command_buffer->buffer_lock->is_mapped == false) {
    return;
  }
  command_buffer->buffer_lock->is_mapped = false;
  command_buffer->buffer_lock->is_mapped_cv.notify_all();
  // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
  if (!glUnmapNamedBuffer(command_buffer->id)) {
    DEBUG_EXPR(CheckGLError());
  }
}

CommandBuffer* GLBuffers::_CreateCommandBuffer() {
  GLuint id;
  // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
  glGenBuffers(1, &id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, id);
  glBufferStorage(GL_DRAW_INDIRECT_BUFFER, COMMAND_STORAGE_SIZE, NULL, flags);

  auto buffer_lock = new BufferLock();

  DrawElementsIndirectCommand* ptr = (DrawElementsIndirectCommand*)glMapNamedBufferRange(
    id, 0, COMMAND_STORAGE_SIZE, flags);
  buffer_lock->is_mapped = true;

  DEBUG_EXPR(CheckGLError());
  return new CommandBuffer{ id, ptr, buffer_lock };
}

void GLBuffers::_SyncGPUBufAndUnmap() {
  WaitGPU(fence_sync);
  if (is_need_barrier) {
    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    is_need_barrier = false;
  }
  _UnmapBuffer();
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
                // command_buffer).

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
  index_ptr = (unsigned int*)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
    INDEX_STORAGE_SIZE, flags);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
  vertex_ptr = (Vertex*)glMapBufferRange(GL_ARRAY_BUFFER, 0,
    VERTEX_STORAGE_SIZE, flags);

  glGenBuffers(1, &uniforms_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, MESH_UNIFORMS_STORAGE_SIZE, NULL, flags);
  uniforms_ptr = (MeshUniform*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, MESH_UNIFORMS_STORAGE_SIZE, flags);

  glGenBuffers(1, &blend_textures_by_mesh_id_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, blend_textures_by_mesh_id_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, BLEND_TEXTURES_BY_MESH_ID_SIZE, NULL, flags);
  blend_textures_by_mesh_id_ptr = (BlendTexturesByMeshIdUniform*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, BLEND_TEXTURES_BY_MESH_ID_SIZE, flags);

  glGenBuffers(1, &texture_handle_by_texture_id_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE, NULL, flags);
  texture_handle_by_texture_id_ptr = (GLuint64*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
    TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE, flags);

  glGenBuffers(1, &light_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, LIGHT_STORAGE_SIZE, NULL, flags);
  light_ptr = (LightsUniform*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
    LIGHT_STORAGE_SIZE, flags);

  // Font buffers
  glGenBuffers(1, &font_texture_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_texture_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, FONT_TEXTURE_STORAGE_SIZE, NULL,
    flags);
  font_texture_ptr = (GLuint64*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, FONT_TEXTURE_STORAGE_SIZE, flags);
  glGenBuffers(1, &uniforms_3d_overlay_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, UNIFORMS_3D_OVERLAY_STORAGE_SIZE, NULL,
    flags);
  uniforms_3d_overlay_ptr = (UniformData3DOverlay*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, UNIFORMS_3D_OVERLAY_STORAGE_SIZE, flags);
  // Font UI buffers
  glGenBuffers(1, &uniforms_ui_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, UNIFORMS_UI_STORAGE_SIZE, NULL,
    flags);
  uniforms_ui_ptr = (UniformDataUI*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, UNIFORMS_UI_STORAGE_SIZE, flags);

  glGenBuffers(1, &uniforms_controller_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_controller_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ControllerUniformData), NULL,
    flags);
  uniforms_controller_ptr = (ControllerUniformData*)glMapBufferRange(
    GL_SHADER_STORAGE_BUFFER, 0, sizeof(ControllerUniformData), flags);

  command_buffer = _CreateCommandBuffer();
}

void GLBuffers::BindVAOandBuffers() {
  _BindCommandBuffer();
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
  }
  if ((used_buffers & BufferType::VERTEX) &&
    !(bound_buffers & BufferType::VERTEX)) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  }
  if ((used_buffers & BufferType::UNIFORMS) &&
    !(bound_buffers & BufferType::UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UNIFORMS_LOC, uniforms_buffer);
  }

  if ((used_buffers & BufferType::BLEND_TEXTURES_BY_MESH_ID) &&
    !(bound_buffers & BufferType::BLEND_TEXTURES_BY_MESH_ID)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, blend_textures_by_mesh_id_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BLEND_TEXTURES_BY_MESH_ID_LOC, blend_textures_by_mesh_id_buffer);
  }

  if ((used_buffers & BufferType::TEXTURE) &&
    !(bound_buffers & BufferType::TEXTURE)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_LOC,
      texture_handle_by_texture_id_buffer);
  }
  if ((used_buffers & BufferType::LIGHT) &&
    !(bound_buffers & BufferType::LIGHT)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_UNIFORM_LOC,
      light_buffer);
  }
  if ((used_buffers & BufferType::INDEX) &&
    !(bound_buffers & BufferType::INDEX)) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  }
  if ((used_buffers & BufferType::FONT_TEXTURE) &&
    !(bound_buffers & BufferType::FONT_TEXTURE)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_texture_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, FONT_TEXTURE_UNIFORM_LOC,
      font_texture_buffer);
  }
  if ((used_buffers & BufferType::UI_UNIFORMS) &&
    !(bound_buffers & BufferType::UI_UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, FONT_UNIFORMS_LOC,
      uniforms_3d_overlay_buffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UI_UNIFORM_LOC,
      uniforms_ui_buffer);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_controller_buffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, CONTROLLER_UNIFORM_LOC,
      uniforms_controller_buffer);
  }
}

void GLBuffers::Dispose() {
  _SyncGPUBufAndUnmap();

  glDisableVertexAttribArray(0);
  glDeleteVertexArrays(1, &mesh_VAO);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glDeleteBuffers(1, &vertex_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glDeleteBuffers(1, &index_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &texture_handle_by_texture_id_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &light_buffer);
  DEBUG_EXPR(CheckGLError());

  _DeleteCommandBuffer(command_buffer);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_ui_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_3d_overlay_buffer);
  DEBUG_EXPR(CheckGLError());

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_texture_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &font_texture_buffer);
  DEBUG_EXPR(CheckGLError());
}
