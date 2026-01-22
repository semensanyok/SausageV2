#include "GLBuffers.h"

void GLBuffers::AddUsedBuffers(BufferType::BufferTypeFlag used_buffers) {
  this->used_buffers |= used_buffers;
}

void GLBuffers::InitBuffers() {
  mesh_uniform_ptr = CreateBufferNumberPool<UniformDataMesh>(MESH_UNIFORMS_STORAGE_SIZE, MAX_MESHES_INSTANCES, GL_SHADER_STORAGE_BUFFER);
  mesh_static_uniform_ptr = CreateBufferNumberPool<UniformDataMeshStatic>(MESH_STATIC_UNIFORMS_STORAGE_SIZE, MAX_MESHES_STATIC_INSTANCES, GL_SHADER_STORAGE_BUFFER);
  mesh_terrain_uniform_ptr = CreateBufferNumberPool<UniformDataMeshTerrain>(MESH_TERRAIN_UNIFORMS_STORAGE_SIZE, MAX_MESHES_TERRAIN, GL_SHADER_STORAGE_BUFFER);
  texture_handle_by_texture_id_ptr = CreateBufferNumberPool<GLuint64>(TEXTURE_HANDLE_BY_TEXTURE_ID_STORAGE_SIZE, MAX_TEXTURE, GL_SHADER_STORAGE_BUFFER);
  light_ptr = CreateBufferNumberPool<LightsUniform>(LIGHT_STORAGE_SIZE, MAX_LIGHTS, GL_SHADER_STORAGE_BUFFER);
  uniforms_3d_overlay_ptr = CreateBufferNumberPool<UniformDataOverlay3D>(UNIFORM_OVERLAY_3D_STORAGE_SIZE, MAX_3D_OVERLAY_INSTANCES, GL_SHADER_STORAGE_BUFFER);
  uniforms_ui_ptr = CreateBufferNumberPool<UniformDataUI>(UNIFORM_UI_STORAGE_SIZE, MAX_UI_INSTANCES, GL_SHADER_STORAGE_BUFFER);
  uniforms_controller_ptr = CreateBufferNumberPool<ControllerUniformData>(UNIFORM_CONTROLLER_SIZE, 1, GL_SHADER_STORAGE_BUFFER);

  terrain_texture_instance_slots = new Arena({0,TERRAIN_MAX_TEXTURES,0});
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
  if ((used_buffers & BufferType::MESH_TERRAIN_UNIFORMS) &&
    !(bound_buffers & BufferType::MESH_TERRAIN_UNIFORMS)) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, mesh_terrain_uniform_ptr->buffer_id);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, MESH_TERRAIN_UNIFORMS_LOC, mesh_terrain_uniform_ptr->buffer_id);
    bound_buffers |= BufferType::MESH_TERRAIN_UNIFORMS;
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
