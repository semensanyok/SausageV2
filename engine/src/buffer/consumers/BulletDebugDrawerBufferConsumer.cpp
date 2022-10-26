#include "BulletDebugDrawerBufferConsumer.h"

void BulletDebugDrawerBufferConsumer::BufferMeshData(vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors)
{
  shared_ptr<MeshLoadData> load_data = mesh_manager->CreateLoadData(vertices, indices, colors);
  buffer->BufferMeshData(mesh, load_data);
}

void BulletDebugDrawerBufferConsumer::Init() {
  BufferConsumer::Init();
  mesh = mesh_manager->CreateMeshData();
  buffer->AllocateStorage(mesh->slots, vertices_size, indices_size);
}

void BulletDebugDrawerBufferConsumer::Reset() { Init(); }
