#include "BulletDebugDrawerBufferConsumer.h"

void BulletDebugDrawerBufferConsumer::BufferMeshData(vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors)
{
    shared_ptr<MeshLoadData> load_data = mesh_manager->CreateMesh(vertices, indices, colors);
    buffer->BufferMeshData(mesh, load_data, vertex_total, index_total, meshes_total,
      margins, SausageDefaults::DEFAULT_MESH_DATA_VECTOR);
}

void BulletDebugDrawerBufferConsumer::Init() {
    margins = buffer->RequestStorage(BufferSettings::Margins::DEBUG_PHYS_VERTEX_PART, BufferSettings::Margins::DEBUG_PHYS_INDEX_PART);
    vertex_total = margins.start_vertex;
    index_total = margins.start_index;
    mesh = mesh_manager->CreateMeshData();
}

void BulletDebugDrawerBufferConsumer::Reset() {
    mesh = nullptr;
    Init();
}
