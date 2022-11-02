#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename TRANSFORM_TYPE>
class BufferConsumer {
protected:
  BufferStorage* buffer;
  BufferType::BufferTypeFlag used_buffers;
public:
  BufferConsumer(BufferStorage* buffer,
    BufferType::BufferTypeFlag used_buffers) :
      buffer{ buffer }
  {
  }
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size,
    unsigned long num_instances) {
    return buffer->AllocateStorage(out_slots, vertices_size, indices_size);
  };

  bool AllocateInstanceSlot(
    MeshDataSlots& out_slots,
    unsigned long num_instances) {
    return buffer->AllocateInstanceSlot<MESH_TYPE>(out_slots, num_instances);
  }
  inline void BufferTransform(const BufferInstanceOffset* offset, const TRANSFORM_TYPE& transform) {
    buffer->BufferTransform<TRANSFORM_TYPE, MESH_TYPE>(offset, transform);
  }
  inline void BufferTexture(const BufferInstanceOffset& mesh, const TEXTURE_ARRAY_TYPE& texture) {
    return buffer->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(mesh, texture);
  }
  void Init()
  {
    buffer->AddUsedBuffers(used_buffers);
  }
  void PreDraw() {
  };
  void PostDraw() {
  };
};
