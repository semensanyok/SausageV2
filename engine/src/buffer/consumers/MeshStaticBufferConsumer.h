#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "MeshManager.h"
#include "GLVertexAttributes.h"
#include "MeshDataBufferConsumerShared.h"

class MeshStaticBufferConsumer : public MeshStaticBufferConsumerSharedT {

public:

  MeshStaticBufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    TextureManager* texture_manager
  )
    : MeshDataBufferConsumerShared(vertex_attributes, mesh_manager, texture_manager, BufferType::MESH_STATIC_BUFFERS) {
  };
};
