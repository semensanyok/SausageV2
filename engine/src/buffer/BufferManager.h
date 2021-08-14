#pragma once

#include "../sausage.h"
#include "consumers/MeshDataBufferConsumer.h"	
#include "BufferStorage.h"
#include "consumers/BulletDebugDrawerBufferConsumer.h"
#include "consumers/FontBufferConsumer.h"

class BufferManager {
public:
	BufferStorage* storage;
	MeshDataBufferConsumer* mesh_data_buffer;
	FontBufferConsumer* font_buffer;
	BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer;
	BufferManager() {}
	void Init() {
		storage = new BufferStorage();
		storage->InitMeshBuffers();
		mesh_data_buffer = new MeshDataBufferConsumer(storage);
		mesh_data_buffer->Init();
		font_buffer = new FontBufferConsumer(storage);
		font_buffer->Init();
		bullet_debug_drawer_buffer = new BulletDebugDrawerBufferConsumer(storage);
		bullet_debug_drawer_buffer->Init();
	}
	void Reset() {
		mesh_data_buffer->Reset();
		font_buffer->Reset();
		bullet_debug_drawer_buffer->Reset();
		storage->Reset();
	}
	//void _DisposeBuffer() {
	//	renderer->RemoveBuffer(buffer);
	//	buffer->Dispose();
	//	delete buffer;
	//}
};