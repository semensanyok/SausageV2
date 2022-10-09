#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    DrawElementsIndirectCommand& command,
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(mesh, load_data);
  buffer->BufferTransform((MeshData*)mesh);
}

bool MeshDataBufferConsumer::RequestBuffersOffsets(
  MeshDataBase* out_mesh,
  unsigned long vertices_size,
  unsigned long indices_size) {
  return buffer->RequestBuffersOffsets(out_mesh, vertices_size, indices_size);
}

void MeshDataBufferConsumer::ReleaseStorage(MeshDataBase* mesh) {
  buffer->ReleaseStorage(mesh);
}

void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

void MeshDataBufferConsumer::BufferTransform(vector<MeshData*>& meshes) {
  buffer->BufferTransform(meshes);
}

void MeshDataBufferConsumer::BufferTransform(MeshData* mesh) {
  buffer->BufferTransform(mesh);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}

void MeshDataBufferConsumer::BufferMeshTexture(MeshData* mesh) {
  buffer->BufferMeshTexture(mesh);
}

void MeshDataBufferConsumer::BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
  buffer->BufferBoneTransform(bones_transforms);
}

/**
  * not used currently
  * @brief for automatic optimization/instansing of meshes, loaded from scene file
  */
void MeshDataBufferConsumer::SetBaseMeshForInstancedCommand(
    vector<MeshDataBase*>& load_data_meshes,
    vector<shared_ptr<MeshLoadData>>& load_data,
    vector<MaterialTexNames>& load_data_textures) {
  throw std::logic_error("Not implemented");
  //unordered_map<size_t, int> instanced_data_lookup;
  //for (int i = 0; i < load_data.size(); i++) {
  //  auto mesh = load_data[i].get();
  //  hash<MaterialTexNames> tex_hash;
  //  auto key =
  //    tex_hash(load_data_textures[i])
  //    + mesh->vertices.size()
  //    + mesh->indices.size();
  //  auto base_mesh_ptr = instanced_data_lookup.find(key);
  //  if (base_mesh_ptr == instanced_data_lookup.end()) {
  //    instanced_data_lookup[key] = i;
  //    continue;
  //  }
  //  auto base_mesh_ind = (*base_mesh_ptr).second;
  //  auto base_mesh_data = load_data_meshes[base_mesh_ind];
  //  auto mesh_data = load_data_meshes[i];

  //  //auto& instance_count = base_mesh_data->instance_count;
  //  mesh_data->instance_id = draw_call_manager->AddInstanceGetInstanceId(mesh_data);
  //  DEBUG_ASSERT(mesh_data->instance_id != -1);
  //  mesh_data->base_mesh = load_data_meshes[base_mesh_ind];
  //}
}
