#include "BufferStorage.h"

BufferMargins BufferStorage::RequestStorage(float buffer_part_percent_vertex,
                                            float buffer_part_percent_index) {
  if (buffer_part_percent_vertex + allocated_percent_vertex > 1.0) {
    LOG((
            ostringstream()
            << "BufferStorage::RequestStorage failed for vertex. requested '"
            << buffer_part_percent_vertex << "' + allocated '"
            << allocated_percent_vertex << "' > 1")
            .str());
    return {};
  }
  if (buffer_part_percent_index + allocated_percent_index > 1.0) {
    LOG((
            ostringstream()
            << "BufferStorage::RequestStorage failed for index. requested '"
            << buffer_part_percent_index << "' + allocated '"
            << allocated_percent_index << "' > 1")
            .str());
    return {};
  }
  auto start_vertex = allocated_percent_vertex * MAX_VERTEX;
  auto start_index = allocated_percent_index * MAX_INDEX;
  auto end_vertex =
      (allocated_percent_vertex + buffer_part_percent_vertex) * MAX_VERTEX;
  auto end_index =
      (allocated_percent_index + buffer_part_percent_index) * MAX_INDEX;

  BufferMargins margins = BufferMargins(start_vertex, end_vertex, start_index, end_index);
  allocated_percent_vertex += buffer_part_percent_vertex;
  allocated_percent_index += buffer_part_percent_index;
  return margins;
}

// map buffers that updated asyncronously
void BufferStorage::MapBuffers() {
  for (auto buf : command_buffers) {
    lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
    MapBuffer(buf);
  }
}
void BufferStorage::MapBuffer(CommandBuffer *buf) {
  if (buf->buffer_lock->is_mapped == true) {
    return;
  }
  // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
  buf->ptr = (DrawElementsIndirectCommand *)glMapNamedBufferRange(
      buf->id, 0, buf->size * sizeof(DrawElementsIndirectCommand), flags);
  mapped_command_buffers[buf->id] = buf;
  buf->buffer_lock->is_mapped = true;
  buf->buffer_lock->is_mapped_cv.notify_all();
}

void BufferStorage::WaitGPU(GLsync fence_sync, const source_location &location) {
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
    waitDuration = 1000000000; // one second in nanoseconds
  }
  glDeleteSync(fence_sync);
  fence_sync = 0;
}
void BufferStorage::SyncGPUBufAndUnmap() {
  WaitGPU(fence_sync);
  if (is_need_barrier) {
    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);
    is_need_barrier = false;
  }
  for (auto &buf : mapped_command_buffers) {
    lock_guard<mutex> data_lock(buf.second->buffer_lock->data_mutex);
    UnmapBuffer(buf.second);
  }
  mapped_command_buffers.clear();
}
void BufferStorage::UnmapBuffer(CommandBuffer *buf) {
  if (buf->buffer_lock->is_mapped == false) {
    return;
  }
  // MUST unmap GL_DRAW_INDIRECT_BUFFER. GL_INVALID_OPERATION otherwise.
  if (!glUnmapNamedBuffer(buf->id)) {
    CheckGLError();
  }
  buf->buffer_lock->is_mapped = false;
  buf->buffer_lock->is_mapped_cv.notify_all();
}
CommandBuffer *BufferStorage::CreateCommandBuffer(unsigned int size) {
  auto buf = new CommandBuffer();
  buf->size = size;
  buf->buffer_lock = new BufferLock();
  buf->buffer_lock->is_mapped = false;

  // MUST unmap INDIRECT DRAW pointer after buffering. Hence - map on demand.
  glGenBuffers(1, &buf->id);
  glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf->id);
  glBufferStorage(GL_DRAW_INDIRECT_BUFFER,
                  size * sizeof(DrawElementsIndirectCommand), NULL, flags);
  CheckGLError();
  return buf;
}
void BufferStorage::ActivateCommandBuffer(CommandBuffer *buf) {
  lock_guard<mutex> data_lock(buf->buffer_lock->data_mutex);
  command_buffers.push_back(buf);
  MapBuffer(buf);
}
void BufferStorage::RemoveCommandBuffer(CommandBuffer *to_remove) {
  lock_guard<mutex> data_lock(to_remove->buffer_lock->data_mutex);
  auto cur_buf = command_buffers.begin();
  while (cur_buf != command_buffers.end()) {
    if (*cur_buf == to_remove) {
      command_buffers.erase(cur_buf);
      mapped_command_buffers.erase(to_remove->id);
      //UnmapBuffer(to_remove);
      break;
    }
    cur_buf++;
  }
}
void BufferStorage::DeleteCommandBuffer(CommandBuffer *to_delete) {
  lock_guard<mutex> data_lock(to_delete->buffer_lock->data_mutex);
  auto cur_buf = command_buffers.begin();
  while (cur_buf != command_buffers.end()) {
    if (*cur_buf == to_delete) {
      command_buffers.erase(cur_buf);
      mapped_command_buffers.erase(to_delete->id);
      UnmapBuffer(to_delete);
      glDeleteBuffers(1, &(to_delete->id));
      break;
    }
    cur_buf++;
  }
  delete to_delete->buffer_lock;
  delete to_delete;
}
int BufferStorage::AddCommands(
    vector<DrawElementsIndirectCommand> &active_commands, CommandBuffer *buf,
    int command_offset) {
  unique_lock<mutex> data_lock(buf->buffer_lock->data_mutex);
  if (!buf->buffer_lock->is_mapped) {
    buf->buffer_lock->Wait(data_lock);
  }
  auto mapped_buffer = mapped_command_buffers.find(buf->id);
  int command_start = command_offset == -1 ? 0 : command_offset;
  if (mapped_buffer != mapped_command_buffers.end()) {
    {
      memcpy(&(mapped_buffer->second->ptr[command_start]),
             active_commands.data(),
             active_commands.size() * sizeof(DrawElementsIndirectCommand));
    }
    return command_start;
  }
  return -1;
}
int BufferStorage::AddCommand(DrawElementsIndirectCommand &command,
                              CommandBuffer *buf, int command_offset) {
  unique_lock<mutex> data_lock(buf->buffer_lock->data_mutex);
  if (!buf->buffer_lock->is_mapped) {
    buf->buffer_lock->Wait(data_lock);
  }
  auto mapped_buffer = mapped_command_buffers.find(buf->id);
  if (mapped_buffer != mapped_command_buffers.end()) {
    int command_start = command_offset == -1 ? 0 : command_offset;
    buf->ptr[command_start] = command;
    return command_start;
  }
  return -1;
}
void BufferStorage::BufferBoneTransform(
    map<unsigned int, mat4> &id_to_transform) {
  for (auto &id_trans : id_to_transform) {
    uniforms_ptr->bones_transforms[id_trans.first] = id_trans.second;
  }
  is_need_barrier = true;
}
void BufferStorage::BufferBoneTransform(Bone *bone, mat4 &trans,
                                        unsigned int num_bones) {
  for (size_t i = 0; i < num_bones; i++) {
    uniforms_ptr->bones_transforms[bone[i].id] = trans;
  }
  is_need_barrier = true;
}
void BufferStorage::BufferTransform(MeshData *mesh) {
  uniforms_ptr->transforms[mesh->transform_offset + mesh->instance_id] =
      mesh->transform;
  if (mesh->instance_id == 0) {
    uniforms_ptr->transform_offset[mesh->buffer_id + mesh->instance_id] =
        mesh->transform_offset;
  }
  is_need_barrier = true;
}
void BufferStorage::BufferTransform(vector<MeshData *> &mesh) {
  for (int i = 0; i < mesh.size(); i++) {
    BufferTransform(mesh[i]);
  }
  // memcpy(&mvp_ptr[command_total], mvps.data(), mvps.size() * sizeof(mat4));
  is_need_barrier = true;
}
void BufferStorage::BufferLights(vector<Light *> &lights) {

  if (lights.size() > MAX_LIGHTS) {
    LOG((ostringstream() << "ERROR BufferLights. max lights buffer size="
                         << MAX_LIGHTS << " requested=" << lights.size())
            .str());
    return;
  }
  light_ptr->num_lights = lights.size();
  for (int i = 0; i < lights.size(); i++) {
    light_ptr->lights[i] = *lights[i];
    // memcpy(light_ptr->lights, lights.data(), lights.size() * sizeof(Light));
  }
  is_need_barrier = true;
}

void BufferStorage::BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data,
    unsigned long& vertex_total,
    unsigned long& index_total,
    unsigned long& meshes_total,
    BufferMargins& margins,
    bool is_transform_used)
{
    vector<MeshData*> instances;
    for (int i = 0; i < load_data.size(); i++) {
        BufferMeshData(load_data[i], vertex_total, index_total, meshes_total, margins, instances, is_transform_used);
    }
    for (auto& instance : instances) {
        instance->buffer_id = instance->base_mesh->buffer_id;
        instance->transform_offset = instance->base_mesh->transform_offset;
    }
    // ids_ptr is SSBO. call after SSBO write (GL_SHADER_STORAGE_BUFFER).
    is_need_barrier = true;
    CheckGLError();
}
void BufferStorage::BufferMeshData(shared_ptr<MeshLoadData> load_data,
    unsigned long& vertex_total,
    unsigned long& index_total,
    unsigned long& meshes_total,
    BufferMargins& margins,
    vector<MeshData*>& instances,
    bool is_transform_used)
{
    auto raw = load_data.get();
    auto& mesh = raw->mesh;
    bool is_instance = mesh->base_mesh != nullptr;
    mesh->buffer = this;
    if (is_instance) {
        instances.push_back(mesh);
        return;
    }
    auto& vertices = raw->vertices;
    auto& indices = raw->indices;
    // if offset initialized - reload data. (if vertices/indices size > existing - will corrupt other meshes)
    bool is_new_mesh = mesh->buffer_id < 0;

    bool is_vertex_offset_provided = mesh->vertex_offset >= 0;
    bool is_index_offset_provided = mesh->index_offset >= 0;
    if (!is_vertex_offset_provided) {
        mesh->vertex_offset = vertex_total;
    }
    if (!is_index_offset_provided) {
        mesh->index_offset = index_total;
    }

    if (vertices.size() + mesh->vertex_offset > margins.end_vertex) {
        LOG((ostringstream() << "ERROR BufferMeshData allocation. vertices total=" << vertex_total << "asked=" << vertices.size()
            << "max=" << margins.end_vertex << " vertex offset=" << mesh->vertex_offset).str());
        return;
    }
    if (indices.size() + mesh->index_offset > MAX_INDEX) {
        LOG((ostringstream() << "ERROR BufferMeshData allocation. indices total=" << index_total << "asked=" << indices.size()
            << "max=" << margins.end_index << " index offset=" << mesh->index_offset).str());
        return;
    }
    if (is_new_mesh) {
        mesh->buffer_id = meshes_total++;
    }
    DrawElementsIndirectCommand& command = mesh->command;
    command.count = indices.size();
    command.instanceCount = raw->instance_count;
    command.firstIndex = mesh->index_offset;
    command.baseVertex = mesh->vertex_offset;
    command.baseInstance = mesh->buffer_id;
    // copy to GPU
    memcpy(&vertex_ptr[mesh->vertex_offset], vertices.data(), vertices.size() * sizeof(Vertex));
    memcpy(&index_ptr[mesh->index_offset], indices.data(), indices.size() * sizeof(unsigned int));
    if (mesh->texture != nullptr) {
        texture_ptr[mesh->buffer_id] = mesh->texture->texture_handle_ARB;
    }

    if (!is_vertex_offset_provided) {
        vertex_total += vertices.size();
    }
    if (!is_index_offset_provided) {
        index_total += indices.size();
    }
    if (is_transform_used) {
        mesh->transform_offset = transforms_total;
        if (is_new_mesh) {
            transforms_total += command.instanceCount;
        }
    }
    if (mesh->armature != nullptr && mesh->armature != NULL) {
        int num_bones = mesh->armature->num_bones;
        if (mesh->armature->bones != NULL) {
            auto identity = mat4(1);
            BufferBoneTransform(mesh->armature->bones, identity, mesh->armature->num_bones);
        }
    }
}
void BufferStorage::InitMeshBuffers() {
  glGenVertexArrays(1, &mesh_VAO);
  glBindVertexArray(
      mesh_VAO); // MUST be bound before glBindBuffer(GL_DRAW_INDIRECT_BUFFER,
                 // command_buffer).

  glGenBuffers(1, &index_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, INDEX_STORAGE_SIZE, NULL, flags);
  index_ptr = (unsigned int *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0,
                                               INDEX_STORAGE_SIZE, flags);

  glGenBuffers(1, &vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glBufferStorage(GL_ARRAY_BUFFER, VERTEX_STORAGE_SIZE, NULL, flags);
  vertex_ptr = (Vertex *)glMapBufferRange(GL_ARRAY_BUFFER, 0,
                                          VERTEX_STORAGE_SIZE, flags);

  glGenBuffers(1, &uniforms_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, UNIFORMS_STORAGE_SIZE, NULL, flags);
  uniforms_ptr = (MeshUniformData *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                                 UNIFORMS_STORAGE_SIZE, flags);

  glGenBuffers(1, &texture_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, TEXTURE_STORAGE_SIZE, NULL, flags);
  texture_ptr = (GLuint64 *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                             TEXTURE_STORAGE_SIZE, flags);


  glGenBuffers(1, &light_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, LIGHT_STORAGE_SIZE, NULL, flags);
  light_ptr = (Lights *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                         LIGHT_STORAGE_SIZE, flags);
  
  
  // Font buffers
  glGenBuffers(1, &font_texture_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_texture_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, FONT_TEXTURE_STORAGE_SIZE, NULL, flags);
  font_texture_ptr = (GLuint64 *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
      FONT_TEXTURE_STORAGE_SIZE, flags);
  glGenBuffers(1, &font_uniforms_buffer);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_uniforms_buffer);
  glBufferStorage(GL_SHADER_STORAGE_BUFFER, FONT_UNIFORMS_STORAGE_SIZE, NULL, flags);
  font_uniforms_ptr = (FontUniformData*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
      FONT_UNIFORMS_STORAGE_SIZE, flags);
}

void BufferStorage::BindVAOandBuffers(BufferType::BufferTypeFlag buffers_to_bind) {
  // TODO: separate Font VAO. it needs less data.
  if ((buffers_to_bind & BufferType::MESH_VAO) && !(bound_buffers & BufferType::MESH_VAO)) {
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
  if ((buffers_to_bind & BufferType::VERTEX) && !(bound_buffers & BufferType::VERTEX)) {
      glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  }
  if ((buffers_to_bind & BufferType::UNIFORMS) && !(bound_buffers & BufferType::UNIFORMS)) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, UNIFORMS_LOC, uniforms_buffer);
  }
  if ((buffers_to_bind & BufferType::TEXTURE) && !(bound_buffers & BufferType::TEXTURE)) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, TEXTURE_UNIFORM_LOC, texture_buffer);
  }
  if ((buffers_to_bind & BufferType::LIGHT) && !(bound_buffers & BufferType::LIGHT)) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, LIGHTS_UNIFORM_LOC, light_buffer);
  }
  if ((buffers_to_bind & BufferType::INDEX) && !(bound_buffers & BufferType::INDEX)) {
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  }
  if ((buffers_to_bind & BufferType::FONT_TEXTURE) && !(bound_buffers & BufferType::FONT_TEXTURE)) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_texture_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, FONT_TEXTURE_UNIFORM_LOC, font_texture_buffer);
  }
  if ((buffers_to_bind & BufferType::FONT_UNIFORMS) && !(bound_buffers & BufferType::FONT_UNIFORMS)) {
      glBindBuffer(GL_SHADER_STORAGE_BUFFER, font_uniforms_buffer);
      glBindBufferBase(GL_SHADER_STORAGE_BUFFER, FONT_UNIFORMS_LOC, uniforms_buffer);
  }
}
void BufferStorage::Dispose() {
  SyncGPUBufAndUnmap();
  glDisableVertexAttribArray(0);
  glDeleteVertexArrays(1, &mesh_VAO);
  CheckGLError();
  glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
  glUnmapBuffer(GL_ARRAY_BUFFER);
  glDeleteBuffers(1, &vertex_buffer);
  CheckGLError();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
  glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
  glDeleteBuffers(1, &index_buffer);
  CheckGLError();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, uniforms_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &uniforms_buffer);
  CheckGLError();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, texture_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &texture_buffer);
  CheckGLError();
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, light_buffer);
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  glDeleteBuffers(1, &light_buffer);
  CheckGLError();
  for (auto command_buffer : command_buffers) {
    glDeleteBuffers(1, &(command_buffer->id));
    delete command_buffer;
  }
  command_buffers.clear();
  CheckGLError();
}
