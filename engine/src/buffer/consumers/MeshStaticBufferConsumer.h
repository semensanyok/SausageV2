#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "MeshManager.h"
#include "GLVertexAttributes.h"

class MeshStaticBufferConsumer : public BufferConsumer<BlendTextures, MeshDataStatic, mat4, VertexStatic> {
public:
  MeshStaticBufferConsumer(
    BufferStorage* buffer,
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
    )
    : BufferConsumer(buffer, vertex_attributes, mesh_manager, BufferType::MESH_STATIC_BUFFERS) {
  };
  void ReleaseSlots(MeshDataBase* mesh) override {
    buffer->ReleaseInstanceSlot<MeshDataStatic>(mesh->slots);
    vertex_attributes->ReleaseStorage<VertexStatic>(mesh->slots);
  }
}
