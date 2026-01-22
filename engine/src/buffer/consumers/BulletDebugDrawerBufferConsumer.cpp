#include "BulletDebugDrawerBufferConsumer.h"

void BulletDebugDrawerBufferConsumer::BufferMeshData(vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors)
{
  shared_ptr<MeshLoadData<VertexOutline>> load_data = mesh_manager->CreateLoadData<VertexOutline>(vertices, indices, colors);
  BufferVertices(mesh->slots, load_data);
  mesh->slots.index_slot.used = indices.size();
}

void BulletDebugDrawerBufferConsumer::Init() {
  BufferConsumer::Init();
  mesh = CreateMeshData();
  mesh->slots.IncNumInstancesGetInstanceId();
  AllocateStorage(mesh->slots, vertices_size, indices_size);
}

void BulletDebugDrawerBufferConsumer::Reset() { Init(); }
