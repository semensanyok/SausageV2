#include "BufferManager.h"

void BufferManager::Init() {
	storage = new BufferStorage();
	storage->InitMeshBuffers();
	mesh_data_buffer = new MeshDataBufferConsumer(storage);
	mesh_data_buffer->Init();
	ui_buffer = new UIBufferConsumer(storage, mesh_manager);
	ui_buffer->Init();
  overlay_3d_buffer = new OverlayBufferConsumer3D(storage, mesh_manager);
  overlay_3d_buffer->Init();
  bullet_debug_drawer_buffer = new BulletDebugDrawerBufferConsumer(storage, mesh_manager);
	bullet_debug_drawer_buffer->Init();
}
void BufferManager::Reset() {
	mesh_data_buffer->Reset();
	ui_buffer->Reset();
  overlay_3d_buffer->Reset();
	bullet_debug_drawer_buffer->Reset();
	storage->Reset();
}
//void _DisposeBuffer() {
//	renderer->RemoveBuffer(buffer);
//	buffer->Dispose();
//	delete buffer;
//}
