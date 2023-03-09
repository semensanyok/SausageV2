#include "GLBuffers.h"

void GLBuffers::AddUsedBuffers(BufferType::BufferTypeFlag used_buffers) {
  this->used_buffers |= used_buffers;
}

void GLBuffers::InitBuffers() {
  mesh_uniform_ptr = CreateBufferSlots<UniformDataMesh>(MESH_UNIFORMS_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  mesh_static_uniform_ptr = CreateBufferSlots<UniformDataMeshStatic>(MESH_STATIC_UNIFORMS_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  texture_handle_by_texture_id_ptr = CreateBufferStorageNumberPool<GLuint64>(TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  light_ptr = CreateBufferSlots<LightsUniform>(LIGHT_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_3d_overlay_ptr = CreateBufferSlots<UniformDataOverlay3D>(UNIFORM_OVERLAY_3D_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_ui_ptr = CreateBufferSlots<UniformDataUI>(UNIFORM_UI_STORAGE_SIZE, GL_SHADER_STORAGE_BUFFER);
  uniforms_controller_ptr = CreateBufferSlots<ControllerUniformData>(UNIFORM_CONTROLLER_SIZE, GL_SHADER_STORAGE_BUFFER);
}

void GLBuffers::BindBuffers() {
  if ((used_buffers & BufferType::MESH_UNIFORMS) &&
    !(bound_buffers & BufferType::MESH_UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_uniform_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MESH_UNIFORMS_LOC, mesh_uniform_ptr->buffer_id);
    bound_buffers |= BufferType::MESH_UNIFORMS;
  }
  if ((used_buffers & BufferType::MESH_STATIC_UNIFORMS) &&
    !(bound_buffers & BufferType::MESH_STATIC_UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_static_uniform_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MESH_STATIC_UNIFORMS_LOC, mesh_static_uniform_ptr->buffer_id);
    bound_buffers |= BufferType::MESH_STATIC_UNIFORMS;
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
  GPUSynchronizer::GetInstance()->SyncGPU();
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, mesh_uniform_ptr->buffer_id);
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, texture_handle_by_texture_id_ptr->buffer_id);
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, light_ptr->buffer_id);
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_ui_ptr->buffer_id);
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_3d_overlay_ptr->buffer_id);
  DeleteBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_controller_ptr->buffer_id);
}
