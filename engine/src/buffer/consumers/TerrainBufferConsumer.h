#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "MeshManager.h"
#include "MeshManager.h"

class TerrainBufferConsumer : BufferConsumer {

public:
  TerrainBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager)
    : BufferConsumer(buffer, BufferType::TERRAIN_BUFFERS),
    mesh_manager{ mesh_manager } {}
};
