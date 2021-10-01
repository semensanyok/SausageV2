#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data, bool is_transform_used)
{
    buffer->BufferMeshData(load_data, vertex_total, index_total, meshes_total, margins, is_transform_used);
}

void MeshDataBufferConsumer::Init() {
    margins = buffer->RequestStorage(BufferSettings::Margins::MESH_DATA_VERTEX_PART, BufferSettings::Margins::MESH_DATA_INDEX_PART);
    vertex_total = margins.start_vertex;
    index_total = margins.start_index;
}

void MeshDataBufferConsumer::Reset() {
    vertex_total = 0;
    index_total = 0;
    meshes_total = 0;
}

void MeshDataBufferConsumer::SetBaseMeshForInstancedCommand(vector<shared_ptr<MeshLoadData>>& new_meshes) {
    map<size_t, shared_ptr<MeshLoadData>> instanced_data_lookup;
    for (auto& mesh_ptr : new_meshes) {
        auto mesh = mesh_ptr.get();
        auto key = mesh->tex_names.Hash() + mesh->vertices.size() + mesh->indices.size();
        auto base_mesh_ptr = instanced_data_lookup.find(key);
        if (base_mesh_ptr == instanced_data_lookup.end()) {
            instanced_data_lookup[key] = mesh_ptr;
            continue;
        }
        auto base_mesh = (*base_mesh_ptr).second.get();
        auto& instance_count = base_mesh->instance_count;
        mesh->mesh->instance_id = instance_count++;
        mesh->mesh->base_mesh = base_mesh->mesh;
    }
}

void MeshDataBufferConsumer::BufferTransform(vector<MeshData*>& meshes)
{
    buffer->BufferTransform(meshes);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
    buffer->BufferLights(lights);
}
