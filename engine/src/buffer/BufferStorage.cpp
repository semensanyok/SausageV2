#include "BufferStorage.h"

void BufferStorage::BufferCommands(
    vector<DrawElementsIndirectCommand>& active_commands, int command_offset) {
  auto command_ptr = gl_buffers->command_ptr->buffer_ptr;
  unique_lock<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
  if (!command_ptr->buffer_lock->is_mapped) {
    command_ptr->buffer_lock->Wait(data_lock);
  }
  memcpy(&(command_ptr->ptr[command_offset]),
         active_commands.data(),
         active_commands.size() * sizeof(DrawElementsIndirectCommand));
}
void BufferStorage::BufferCommand(DrawElementsIndirectCommand& command, int command_offset) {
  auto command_ptr = gl_buffers->command_ptr->buffer_ptr;
  unique_lock<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
  while (!command_ptr->buffer_lock->is_mapped) {
    command_ptr->buffer_lock->Wait(data_lock);
  }
  command_ptr->ptr[command_offset] = command;
}

void BufferStorage::BufferBoneTransform(
    unordered_map<unsigned int, mat4> &id_to_transform) {
  for (auto &id_trans : id_to_transform) {
    gl_buffers->mesh_uniform_ptr->buffer_ptr->bones_transforms[id_trans.first] = id_trans.second;
  }
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferBoneTransform(Bone *bone, mat4 &trans,
                                        unsigned int num_bones) {
  auto mesh_uniform_ptr = gl_buffers->mesh_uniform_ptr->buffer_ptr;
  for (size_t i = 0; i < num_bones; i++) {
    mesh_uniform_ptr->bones_transforms[bone[i].id] = trans;
  }
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferLights(vector<Light *> &lights) {
  if (lights.size() > MAX_LIGHTS) {
    LOG((ostringstream() << "ERROR BufferLights. max lights buffer size="
                         << MAX_LIGHTS << " requested=" << lights.size())
            .str());
    return;
  }
  gl_buffers->light_ptr->buffer_ptr->num_lights = lights.size();
  for (int i = 0; i < lights.size(); i++) {
    gl_buffers->light_ptr->buffer_ptr->lights[i] = *lights[i];
    // memcpy(light_ptr->lights, lights.data(), lights.size() * sizeof(Light));
  }
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferMeshData(MeshDataSlots& slots,
                                   shared_ptr<MeshLoadData>& load_data) {
  auto mesh_data = load_data.get();
  // storage must be allocated at this point (via AllocateStorage)
  DEBUG_ASSERT(slots.index_slot.count > 0);
  DEBUG_ASSERT(slots.vertex_slot.count > 0);
  DEBUG_ASSERT(slots.index_slot.count >= mesh_data->indices.size());
  DEBUG_ASSERT(slots.vertex_slot.count >= mesh_data->vertices.size());

  auto& vertices = mesh_data->vertices;
  auto& indices = mesh_data->indices;
  // copy to GPU
  memcpy(&gl_buffers->vertex_ptr->buffer_ptr[slots.vertex_slot.offset], vertices.data(),
         vertices.size() * sizeof(Vertex));
  memcpy(&gl_buffers->index_ptr->buffer_ptr[slots.index_slot.offset], indices.data(),
         indices.size() * sizeof(unsigned int));
  gl_buffers->SetSyncBarrier();
}

bool BufferStorage::AllocateStorage(
  MeshDataSlots& out_slots,
  unsigned long vertices_size,
  unsigned long indices_size
) {
  auto vertex_slot = gl_buffers->vertex_ptr->instances_slots.Allocate(vertices_size);
  if (vertex_slot == MemorySlots::NULL_SLOT) {
    LOG("Error RequestStorageSetOffsets vertices slot allocation.");
    return false;
  }
  auto index_slot = gl_buffers->index_ptr->instances_slots.Allocate(indices_size);
  if (index_slot == MemorySlots::NULL_SLOT) {
    gl_buffers->vertex_ptr->instances_slots.Release(vertex_slot);
    LOG("Error RequestStorageSetOffsets indices slot allocation.");
    return false;
  }
  return true;
}

void BufferStorage::ReleaseStorage(MeshDataSlots& out_slots) {
  gl_buffers->vertex_ptr->instances_slots.Release(out_slots.vertex_slot);
  gl_buffers->index_ptr->instances_slots.Release(out_slots.index_slot);
}
////////////// ......InstanceSlot END ///////////////////////////////////////////////////

void BufferStorage::Init() {
  gl_buffers->InitBuffers();
}

Texture* BufferStorage::CreateTextureWithBufferSlot(GLuint gl_texture_id, GLuint64 gl_texture_handle_ARB) {
  Texture* texture = new Texture(
    gl_texture_id,
    gl_texture_handle_ARB,
    gl_buffers->texture_handle_by_texture_id_ptr->Allocate());
  _BufferTextureHandle(texture);
  return texture;
};
void BufferStorage::_BufferTextureHandle(Texture* texture)
{
  gl_buffers->
    texture_handle_by_texture_id_ptr->buffer_ptr[texture->id] = texture->texture_handle_ARB;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::ReleaseTexture(Texture* texture) {
  gl_buffers->
    texture_handle_by_texture_id_ptr->instances_slots.ReleaseNumber(texture->id);
  texture->Dispose();
};

void BufferStorage::BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y) {
  DEBUG_ASSERT(mesh->slots.buffer_id > 0);
  DEBUG_ASSERT(mesh->slots.instances_slot != MemorySlots::NULL_SLOT);
  gl_buffers->
    uniforms_ui_ptr->buffer_ptr->
      min_max_x_y[mesh->slots.instances_slot.offset] = { min_x, max_x, min_y, max_y };
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click) {
  gl_buffers->uniforms_controller_ptr->buffer_ptr->mouse_x = mouse_x;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->mouse_y = mouse_y;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->is_pressed = is_pressed;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->is_click = is_click;
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
