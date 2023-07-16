#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "MeshManager.h"
#include "GLVertexAttributes.h"
#include "MeshDataBufferConsumerShared.h"

class MeshTerrainBufferConsumer : public MeshTerrainBufferConsumerSharedT {

public:

  MeshTerrainBufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    TextureManager* texture_manager
  )
    : MeshDataBufferConsumerShared(vertex_attributes, mesh_manager, texture_manager, BufferType::MESH_TERRAIN_BUFFERS) {
  };

  TerrainBlendTextures* GetTexturesSlot() {
    return buffer->gl_buffers->GetTexturesSlot<TerrainBlendTextures>();
  }
};
