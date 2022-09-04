#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    vector<MeshDataBase*>& load_data_meshes,
    vector<shared_ptr<MeshLoadData>>& load_data) {
  buffer->BufferMeshData(load_data_meshes, load_data, vertex_total, index_total,
                         meshes_total, margins);
  for (auto& mesh : load_data_meshes) {
    buffer->BufferTransform((MeshData*)mesh);
  }
}

void MeshDataBufferConsumer::Init() {
  margins =
      buffer->RequestStorage(BufferSettings::Margins::MESH_DATA_VERTEX_PART,
                             BufferSettings::Margins::MESH_DATA_INDEX_PART);
  vertex_total = margins.start_vertex;
  index_total = margins.start_index;
}

void MeshDataBufferConsumer::Reset() {
  vertex_total = 0;
  index_total = 0;
  meshes_total = 0;
}

void MeshDataBufferConsumer::SetBaseMeshForInstancedCommand(
    vector<MeshDataBase*>& load_data_meshes,
    vector<shared_ptr<MeshLoadData>>& load_data) {
  map<size_t, int> instanced_data_lookup;
  for (int i = 0; i < load_data.size(); i++) {
    auto mesh = load_data[i].get();
    auto key =
        mesh->tex_names->Hash() + mesh->vertices.size() + mesh->indices.size();
    auto base_mesh_ptr = instanced_data_lookup.find(key);
    if (base_mesh_ptr == instanced_data_lookup.end()) {
      instanced_data_lookup[key] = i;
      continue;
    }
    auto base_mesh_ind = (*base_mesh_ptr).second;
    auto base_mesh = load_data[base_mesh_ind];
    auto& instance_count = base_mesh->instance_count;
    auto mesh_data = load_data_meshes[i];
    mesh_data->instance_id = instance_count++;
    mesh_data->base_mesh = load_data_meshes[base_mesh_ind];
  }
}

void MeshDataBufferConsumer::BufferTransform(vector<MeshData*>& meshes) {
  buffer->BufferTransform(meshes);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}
