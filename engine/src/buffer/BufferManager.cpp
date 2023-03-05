#include "BufferManager.h"

void BufferManager::Init() {
  storage = BufferStorage::GetInstance();
  storage->Init();
  command_buffer_manager = new CommandBuffersManager();
  vertex_attributes = new GLVertexAttributes();

  mesh_data_buffer = new MeshDataBufferConsumer(storage, vertex_attributes, mesh_manager);
  mesh_data_buffer->Init();
  ui_buffer = new UIBufferConsumer(storage, vertex_attributes, mesh_manager);
  ui_buffer->Init();
  overlay_3d_buffer = new OverlayBufferConsumer3D(storage, vertex_attributes, mesh_manager);
  overlay_3d_buffer->Init();
  mesh_static_buffer = new MeshStaticBufferConsumer(storage, vertex_attributes, mesh_manager);
  mesh_static_buffer->Init();
}
void BufferManager::Reset() {
  storage->Reset();
  mesh_data_buffer->Reset();
  ui_buffer->Reset();
  overlay_3d_buffer->Reset();
  bullet_debug_drawer_buffer->Reset();
  storage->Reset();
}

BulletDebugDrawerBufferConsumer* BufferManager::GetPhysDebugDrawer() {
  if (bullet_debug_drawer_buffer == nullptr) {
    bullet_debug_drawer_buffer = new BulletDebugDrawerBufferConsumer(storage, vertex_attributes, mesh_manager);
    bullet_debug_drawer_buffer->Init();
  }
  return bullet_debug_drawer_buffer;
}
//void _DisposeBuffer() {
//	renderer->RemoveBuffer(buffer);
//	buffer->Dispose();
//	delete buffer;
//}
