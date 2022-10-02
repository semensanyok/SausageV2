#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    MeshDataBase* load_data_mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(load_data_mesh, load_data);
  buffer->BufferTransform((MeshData*)load_data_mesh);
}


void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

/**
 * @brief for automatic optimization/instansing of meshes, loaded from scene file
*/
void MeshDataBufferConsumer::SetBaseMeshForInstancedCommand(
    vector<MeshDataBase*>& load_data_meshes,
    vector<shared_ptr<MeshLoadData>>& load_data) {
  unordered_map<size_t, int> instanced_data_lookup;
  for (int i = 0; i < load_data.size(); i++) {
    auto mesh = load_data[i].get();
    auto key =
      (mesh->tex_names == nullptr ? 0 : mesh->tex_names->Hash())
      + mesh->vertices.size()
      + mesh->indices.size();
    auto base_mesh_ptr = instanced_data_lookup.find(key);
    if (base_mesh_ptr == instanced_data_lookup.end()) {
      instanced_data_lookup[key] = i;
      continue;
    }
    auto base_mesh_ind = (*base_mesh_ptr).second;
    auto base_mesh_data = load_data_meshes[base_mesh_ind];
    auto mesh_data = load_data_meshes[i];
    auto& instance_count = base_mesh_data->instance_count;
    mesh_data->instance_id = instance_count++;
    mesh_data->base_mesh = load_data_meshes[base_mesh_ind];
  }
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
