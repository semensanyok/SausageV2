#include "BufferManager.h"

void BufferManager::Init() {
  buffer_storage->Init();
  //command_buffer_manager = new CommandBuffersManager();
  //command_buffer_manager->InitBuffers();
  vertex_attributes = new GLVertexAttributes();
  vertex_attributes->InitVAO();

  mesh_data_buffer = new MeshDataBufferConsumer(vertex_attributes, mesh_manager, texture_manager);
  mesh_data_buffer->Init();
  mesh_static_buffer = new MeshStaticBufferConsumer(vertex_attributes, mesh_manager, texture_manager);
  mesh_static_buffer->Init();
  mesh_terrain_buffer = new MeshTerrainBufferConsumer(vertex_attributes, mesh_manager, texture_manager);
  mesh_terrain_buffer->Init();
  ui_buffer = new UIBufferConsumer(vertex_attributes, mesh_manager);
  ui_buffer->Init();
  overlay_3d_buffer = new OverlayBufferConsumer3D(vertex_attributes, mesh_manager);
  overlay_3d_buffer->Init();
  mesh_outline_buffer = new MeshDataOutlineBufferConsumer(vertex_attributes, mesh_manager);
  mesh_outline_buffer->Init();
}
void BufferManager::Reset() {
  buffer_storage->Reset();
  mesh_data_buffer->Reset();
  ui_buffer->Reset();
  overlay_3d_buffer->Reset();
  bullet_debug_drawer_buffer->Reset();
}

BulletDebugDrawerBufferConsumer* BufferManager::GetPhysDebugDrawer() {
  if (bullet_debug_drawer_buffer == nullptr) {
    bullet_debug_drawer_buffer = new BulletDebugDrawerBufferConsumer(vertex_attributes, mesh_manager);
    bullet_debug_drawer_buffer->Init();
  }
  return bullet_debug_drawer_buffer;
}
//void _DisposeBuffer() {
//	renderer->RemoveBuffer(buffer);
//	buffer->Dispose();
//	delete buffer;
//}
