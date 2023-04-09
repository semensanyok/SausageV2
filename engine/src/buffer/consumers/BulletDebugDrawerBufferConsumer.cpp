#include "BulletDebugDrawerBufferConsumer.h"

void BulletDebugDrawerBufferConsumer::BufferMeshData(vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors)
{
  shared_ptr<MeshLoadData<VertexOutline>> load_data = mesh_manager->CreateLoadData<VertexOutline>(vertices, indices, colors);
  BufferVertices(mesh->slots, load_data);
  mesh->slots.index_slot.used = indices.size();
  mesh->slots.instances_slot.used = 1;
}

void BulletDebugDrawerBufferConsumer::Init() {
  BufferConsumer::Init();
  mesh = mesh_manager->CreateMeshDataOutline();
  AllocateStorage(mesh->slots, vertices_size, indices_size);
  // preallocated indices_size, but `MeshDataSlots.used` will be set after each draw call after buffering
  // as it controls number of vertices to read from buffer in
  // DrawCallManager#_SetToCommandWithOffsets(...) ... command.count = mesh_slots.index_slot.used;
  mesh->slots.index_slot.used = 0;
}

void BulletDebugDrawerBufferConsumer::Reset() { Init(); }
