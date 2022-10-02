#include "BufferStorage.h"

void BufferStorage::BufferCommands(
    vector<DrawElementsIndirectCommand>& active_commands, int command_offset) {
  auto command_buffer = gl_buffers->command_buffer;
  unique_lock<mutex> data_lock(command_buffer->buffer_lock->data_mutex);
  if (!command_buffer->buffer_lock->is_mapped) {
    command_buffer->buffer_lock->Wait(data_lock);
  }
  memcpy(&(command_buffer->ptr[command_offset]),
         active_commands.data(),
         active_commands.size() * sizeof(DrawElementsIndirectCommand));
}
void BufferStorage::BufferCommand(DrawElementsIndirectCommand& command, int command_offset) {
  auto command_buffer = gl_buffers->command_buffer;
  unique_lock<mutex> data_lock(command_buffer->buffer_lock->data_mutex);
  while (!command_buffer->buffer_lock->is_mapped) {
    command_buffer->buffer_lock->Wait(data_lock);
  }
  command_buffer->ptr[command_offset] = command;
}
void BufferStorage::BufferBoneTransform(
    unordered_map<unsigned int, mat4> &id_to_transform) {
  for (auto &id_trans : id_to_transform) {
    gl_buffers->uniforms_ptr->bones_transforms[id_trans.first] = id_trans.second;
  }
  gl_buffers->SetSyncBarrier();
}
void BufferStorage::BufferBoneTransform(Bone *bone, mat4 &trans,
                                        unsigned int num_bones) {
  auto uniforms_ptr = gl_buffers->uniforms_ptr;
  for (size_t i = 0; i < num_bones; i++) {
    uniforms_ptr->bones_transforms[bone[i].id] = trans;
  }
  gl_buffers->SetSyncBarrier();
}
void BufferStorage::BufferTransform(MeshData *mesh) {
  auto uniforms_ptr = gl_buffers->uniforms_ptr;
  // TODO: set up transform offset once in BufferMeshData
  //if (mesh->transform_offset == -1) {
  //  mesh->transform_offset = _GetTransformBucket(mesh);
  //}
  gl_buffers->uniforms_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
      mesh->transform;
  if (mesh->instance_id == 0) {
    gl_buffers->uniforms_ptr->transform_offset[mesh->buffer_id] = mesh->transform_offset;
  }
  gl_buffers->SetSyncBarrier();
}
void BufferStorage::BufferTransform(vector<MeshData *> &mesh) {
  for (int i = 0; i < mesh.size(); i++) {
    BufferTransform(mesh[i]);
  }
}
void BufferStorage::BufferLights(vector<Light *> &lights) {
  if (lights.size() > MAX_LIGHTS) {
    LOG((ostringstream() << "ERROR BufferLights. max lights buffer size="
                         << MAX_LIGHTS << " requested=" << lights.size())
            .str());
    return;
  }
  gl_buffers->light_ptr->num_lights = lights.size();
  for (int i = 0; i < lights.size(); i++) {
    gl_buffers->light_ptr->lights[i] = *lights[i];
    // memcpy(light_ptr->lights, lights.data(), lights.size() * sizeof(Light));
  }
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferMeshData(vector<MeshDataBase *> &load_data_meshes,
                                   vector<shared_ptr<MeshLoadData>> &load_data) {
  vector<MeshDataBase *> instances;
  for (int i = 0; i < load_data.size(); i++) {
    BufferMeshData(load_data_meshes[i], load_data[i], instances);
  }
  for (auto &instance : instances) {
    instance->buffer_id = instance->base_mesh->buffer_id;
    instance->transform_offset = instance->base_mesh->transform_offset;
  }
  // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  gl_buffers->SetSyncBarrier();
  DEBUG_EXPR(CheckGLError());
}

void BufferStorage::BufferMeshData(MeshDataBase* mesh,
                                   shared_ptr<MeshLoadData> load_data) {
  auto mesh_data = load_data.get();
  auto& vertices = mesh_data->vertices;
  auto& indices = mesh_data->indices;
  // copy to GPU
  memcpy(&gl_buffers->vertex_ptr[mesh->vertex_slot.offset], vertices.data(),
         vertices.size() * sizeof(Vertex));
  memcpy(&gl_buffers->index_ptr[mesh->index_slot.offset], indices.data(),
         indices.size() * sizeof(unsigned int));
  gl_buffers->SetSyncBarrier();
}

bool BufferStorage::RequestStorageSetOffsets(
  DrawElementsIndirectCommand& out_command,
  MeshDataBase* mesh,
  unsigned long vertices_size,
  unsigned long indices_size
) {
  auto vertex_slot = vertex_arena->Allocate(vertices_size);
  if (vertex_slot == Arena::NULL_SLOT) {
    return false;
  }
  auto index_slot = index_arena->Allocate(indices_size);
  if (index_slot == Arena::NULL_SLOT) {
    vertex_arena->Release(vertex_slot);
    return false;
  }
  mesh->vertex_slot = vertex_slot;
  mesh->index_slot = index_slot;

  out_command.count = indices_size;
  out_command.instanceCount = index_slot.count;
  out_command.firstIndex = index_slot.offset;
  out_command.baseVertex = vertex_slot.offset;
  out_command.baseInstance = mesh->buffer_id;
  return true;
}

void BufferStorage::ReleaseStorage(MeshDataBase* mesh) {
  vertex_arena->Release(mesh->vertex_slot);
  index_arena->Release(mesh->index_slot);
}

void BufferStorage::Init() {
  gl_buffers->InitBuffers();
}

void BufferStorage::BufferTextureHandle(Texture* texture)
{
  gl_buffers->
    texture_handle_by_texture_id_ptr[texture->id] = texture->texture_handle_ARB;
  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferMeshTexture(MeshData* mesh) {
  gl_buffers->
    blend_textures_by_mesh_id_ptr->blend_textures[mesh->transform_offset + mesh->instance_id] =
      mesh->textures;
  //uniforms_ptr->blend_textures[0].textures[0].texture_id = 1;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferFontTexture(MeshDataBase* mesh, Texture* texture) {
  gl_buffers->
    font_texture_ptr[mesh->buffer_id] = texture->texture_handle_ARB;
  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::Buffer3DFontTransform(MeshDataOverlay3D* mesh) {
  gl_buffers->
    uniforms_3d_overlay_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
        mesh->transform;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y) {
  gl_buffers->uniforms_ui_ptr
    ->min_max_x_y[mesh->transform_offset + mesh->instance_id] =
      { min_x, max_x, min_y, max_y };
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferUniformDataUITransform(MeshDataUI* mesh) {
  gl_buffers
    ->uniforms_ui_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
      mesh->transform;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click) {
  gl_buffers->uniforms_controller_ptr->mouse_x = mouse_x;
  gl_buffers->uniforms_controller_ptr->mouse_y = mouse_y;
  gl_buffers->uniforms_controller_ptr->is_pressed = is_pressed;
  gl_buffers->uniforms_controller_ptr->is_click = is_click;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::AddUsedBuffers(BufferType::BufferTypeFlag used_buffers) {
  gl_buffers->AddUsedBuffers(used_buffers);
}

void BufferStorage::PreDraw() {
  gl_buffers->PreDraw();
}

void BufferStorage::PostDraw() {
  gl_buffers->PostDraw();
}
