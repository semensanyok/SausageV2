#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"
#include "MeshDataStruct.h"
#include "GLVertexAttributes.h"


/**
 * @brief Adapter/Interface to Uniforms/vertex arrays
 * @tparam TEXTURE_ARRAY_TYPE 
 * @tparam MESH_TYPE 
 * @tparam TRANSFORM_TYPE 
*/
template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename TRANSFORM_TYPE, typename VERTEX_TYPE>
class BufferConsumer {
public:
  BufferStorage* buffer;
  GLVertexAttributes* vertex_attributes;
  MeshManager* mesh_manager;

  BufferType::BufferTypeFlag used_buffers;
  BufferConsumer(
    BufferStorage* buffer,
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    BufferType::BufferTypeFlag used_buffers
  ) :
    buffer{ buffer },
    vertex_attributes{ vertex_attributes },
    mesh_manager{ mesh_manager },
    used_buffers{ used_buffers }
  {
  }
  void BufferVertices(MeshDataSlots& slots,
                      shared_ptr<MeshLoadData<VERTEX_TYPE>>& load_data) {
    vertex_attributes->BufferVertices<VERTEX_TYPE>(slots, load_data);
  }
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size) {
    return vertex_attributes->AllocateStorage<VERTEX_TYPE>(out_slots, vertices_size, indices_size);
  };
  bool AllocateInstanceSlot(
    MeshDataSlots& out_slots,
    unsigned long num_instances) {
    return buffer->AllocateInstanceSlot<MESH_TYPE>(out_slots, num_instances);
  }
  inline void BufferTransform(BufferInstanceOffset* offset, TRANSFORM_TYPE& transform) {
    buffer->BufferTransform<TRANSFORM_TYPE, MESH_TYPE>(*offset, transform);
  }
  inline void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    return buffer->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(*mesh, texture);
  }
  virtual void ReleaseSlots(MeshDataBase* mesh) {
    // child override to not try/catch or check nptr
    //buffer->ReleaseInstanceSlot<MESH_TYPE>(mesh->slots);
    //vertex_attributes->ReleaseStorage(mesh->slots);
  }
  void Init()
  {
    buffer->AddUsedBuffers(used_buffers);
  }
  void PreDraw() {
  };
  void PostDraw() {
  };
  void Release() {
  };
  void Reset() {

  };
};
