#pragma once

#include "sausage.h"
#include "BufferStorage.h"
#include "MeshDataBufferConsumer.h"
#include "BulletDebugDrawerBufferConsumer.h"

class BufferManager {
	const BufferStorage storage;
	const MeshDataBufferConsumer mesh_data_buffer;
	const FontBufferConsumer font_buffer;
	const BulletDebugDrawerBufferConsumer bullet_debug_drawer_buffer;
public:
	BufferManager() {
		storage = BufferStorage();
		mesh_data_buffer = MeshDataBufferConsumer(storage);
		font_buffer = FontBufferConsumer(storage);
		bullet_debug_drawer_buffer = BulletDebugDrawerBufferConsumer();
	}
};