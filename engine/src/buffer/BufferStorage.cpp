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

void BufferStorage::BufferMeshData(MeshDataBase* mesh,
                                   shared_ptr<MeshLoadData>& load_data) {
  auto mesh_data = load_data.get();
  // storage must be allocated at this point (via AllocateStorage)
  DEBUG_ASSERT(mesh->slots.index_slot.count > 0);
  DEBUG_ASSERT(mesh->slots.vertex_slot.count > 0);
  DEBUG_ASSERT(mesh->slots.index_slot.count >= mesh_data->indices.size());
  DEBUG_ASSERT(mesh->slots.vertex_slot.count >= mesh_data->vertices.size());

  auto& vertices = mesh_data->vertices;
  auto& indices = mesh_data->indices;
  // copy to GPU
  memcpy(&gl_buffers->vertex_ptr[mesh->slots.vertex_slot.offset], vertices.data(),
         vertices.size() * sizeof(Vertex));
  memcpy(&gl_buffers->index_ptr[mesh->slots.index_slot.offset], indices.data(),
         indices.size() * sizeof(unsigned int));
  gl_buffers->SetSyncBarrier();
}

bool BufferStorage::AllocateStorage(
  MeshDataSlots& out_slots,
  unsigned long vertices_size,
  unsigned long indices_size,
  unsigned long num_instances
) {
  auto vertex_slot = vertex_arena->Allocate(vertices_size);
  out_slots.buffer_id = command_slots->ObtainNumber();
  if (vertex_slot == MemorySlots::NULL_SLOT) {
    LOG("Error RequestStorageSetOffsets vertices slot allocation.");
    command_slots->ReleaseNumber(out_slots.buffer_id);
    return false;
  }
  auto index_slot = index_arena->Allocate(indices_size);
  if (index_slot == MemorySlots::NULL_SLOT) {
    vertex_arena->Release(vertex_slot);
    LOG("Error RequestStorageSetOffsets indices slot allocation.");
    command_slots->ReleaseNumber(out_slots.buffer_id);
    return false;
  }
  if (!AllocateInstancesSlot(out_slots, num_instances)) {
    vertex_arena->Release(vertex_slot);
    index_arena->Release(index_slot);
    command_slots->ReleaseNumber(out_slots.buffer_id);
    LOG("Error RequestStorageSetOffsets instances slot allocation.");
    return false;
  }
  return true;
}

bool BufferStorage::AllocateInstancesSlot(
    MeshDataSlots& out_slots,
    unsigned long num_instances) {
  auto instances_slot = instances_slots->Allocate(num_instances);
  if (instances_slot == MemorySlots::NULL_SLOT) {
    LOG("Error AllocateInstancesSlot.");
    return false;
  }
  // because buffer id must be initialized before this call (currently in AllocateStorage)
  DEBUG_ASSERT(out_slots.buffer_id > 0);
  out_slots.instances_slot = instances_slot;

  gl_buffers->uniforms_ptr->base_instance_offset[out_slots.buffer_id] = out_slots.instances_slot.offset;
  gl_buffers->SetSyncBarrier();

  return true;
};

void BufferStorage::ReleaseStorage(MeshDataSlots& out_slots) {
  vertex_arena->Release(out_slots.vertex_slot);
  out_slots.vertex_slot = MemorySlots::NULL_SLOT;
  index_arena->Release(out_slots.index_slot);
  out_slots.index_slot = MemorySlots::NULL_SLOT;
  command_slots->ReleaseNumber(out_slots.buffer_id);
  out_slots.buffer_id = MemorySlots::NULL_SLOT;
  ReleaseInstancesSlot(out_slots);
}

void BufferStorage::ReleaseInstancesSlot(MeshDataSlots& out_slots) {
  instances_slots->Release(out_slots.instances_slot);
  out_slots.instances_slot = MemorySlots::NULL_SLOT;
};

void BufferStorage::Init() {
  gl_buffers->InitBuffers();
}

void BufferStorage::BufferTextureHandle(Texture* texture)
{
  gl_buffers->
    texture_handle_by_texture_id_ptr[texture->id] = texture->texture_handle_ARB;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferTexture(unsigned int instance_offset, Texture* texture) {
  gl_buffers->font_texture_ptr[instance_offset] = texture->texture_handle_ARB;
  gl_buffers->SetSyncBarrier();
}

void BufferStorage::BufferTransform(unsigned int instance_offset, mat4& transform) {
  gl_buffers->uniforms_ptr->transforms[instance_offset] = transform;
  gl_buffers->SetSyncBarrier();
}


void BufferStorage::BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y) {
  DEBUG_ASSERT(mesh->slots.buffer_id > 0);
  DEBUG_ASSERT(mesh->slots.instances_slot != MemorySlots::NULL_SLOT);
  gl_buffers->
    uniforms_ui_ptr->
      min_max_x_y[mesh->slots.instances_slot.offset] = { min_x, max_x, min_y, max_y };
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
