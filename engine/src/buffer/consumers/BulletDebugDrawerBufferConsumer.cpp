#include "BulletDebugDrawerBufferConsumer.h"

void BulletDebugDrawerBufferConsumer::BufferMeshData(vector<vec3>& vertices, vector<unsigned int>& indices, vector<vec3>& colors, bool is_transform_used)
{
    bool is_new_mesh_data = mesh == nullptr;
    shared_ptr<MeshLoadData> load_data = mesh_manager->CreateMesh(vertices, indices, colors, is_new_mesh_data);
    if (is_new_mesh_data) {
        mesh = load_data.get()->mesh;
        mesh->name = "BulletDebugDrawerData";
        mesh->vertex_offset = margins.start_vertex;
        mesh->index_offset = margins.start_index;
    }
    else {
        load_data->mesh = mesh;
    }
    buffer->BufferMeshData(load_data, vertex_total, index_total, meshes_total, margins, SausageDefaults::DEFAULT_MESH_DATA_VECTOR, is_transform_used);
}

void BulletDebugDrawerBufferConsumer::Init() {
    margins = buffer->RequestStorage(BufferSettings::Margins::DEBUG_PHYS_VERTEX_PART, BufferSettings::Margins::DEBUG_PHYS_INDEX_PART);
    vertex_total = margins.start_vertex;
    index_total = margins.start_index;
}

void BulletDebugDrawerBufferConsumer::Reset() {
    delete mesh;
    Init();
}
