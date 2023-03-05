#include "BufferStorage.h"

void BufferStorage::BufferBoneTransform(
    unordered_map<unsigned int, mat4> &id_to_transform) {
  for (auto &id_trans : id_to_transform) {
    gl_buffers->mesh_uniform_ptr->buffer_ptr->bones_transforms[id_trans.first] = id_trans.second;
  }
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
}

void BufferStorage::BufferBoneTransform(Bone *bone, mat4 &trans,
                                        unsigned int num_bones) {
  auto mesh_uniform_ptr = gl_buffers->mesh_uniform_ptr->buffer_ptr;
  for (size_t i = 0; i < num_bones; i++) {
    mesh_uniform_ptr->bones_transforms[bone[i].id] = trans;
  }
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
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
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
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
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
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
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
}

void BufferStorage::BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click) {
  gl_buffers->uniforms_controller_ptr->buffer_ptr->mouse_x = mouse_x;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->mouse_y = mouse_y;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->is_pressed = is_pressed;
  gl_buffers->uniforms_controller_ptr->buffer_ptr->is_click = is_click;
  GPUSynchronizer::GetInstance()->SetSyncBarrier();
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
