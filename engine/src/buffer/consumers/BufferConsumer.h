#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"
#include "MeshDataStruct.h"
#include "GLVertexAttributes.h"
#include "MeshManager.h"


using namespace glm;
using namespace std;

/**
 * @brief Adapter/Interface to Uniforms/vertex arrays
 * @tparam TEXTURE_ARRAY_TYPE 
 * @tparam MESH_TYPE 
 * @tparam TRANSFORM_TYPE 
*/
template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename TRANSFORM_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
class BufferConsumer {
public:
  BufferStorage* buffer;
  GLVertexAttributes* vertex_attributes;
  MeshManager* mesh_manager;

  BufferType::BufferTypeFlag used_buffers;
  BufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    BufferType::BufferTypeFlag used_buffers
  ) :
    buffer{ BufferStorage::GetInstance() },
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
  inline bool AllocateStorage(
    MeshDataSlots& out_slots,
    shared_ptr<MeshLoadData<VERTEX_TYPE>>& load_data) {
    return vertex_attributes->AllocateStorage<VERTEX_TYPE>(out_slots,
      load_data->vertices.size(),
      load_data->indices.size());
  };
  bool AllocateUniformOffset(
    InstanceSlotUpdate* out_slots) {
    return buffer->AllocateUniformOffset<UNIFORM_DATA_TYPE, MESH_TYPE>(out_slots);
  }
  void BufferUniformOffset(BufferInstanceOffset* offset) {
    buffer->BufferUniformOffset<MESH_TYPE>(offset);
  }
  void BufferTransform(BufferInstanceOffset* offset, const TRANSFORM_TYPE& transform) {
    buffer->BufferTransform<TRANSFORM_TYPE, MESH_TYPE>(offset, transform);
  }
  void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    buffer->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(mesh, texture);
  }
  void ReleaseUniformOffset(MeshDataSlots& out_slots) {
    buffer->ReleaseUniformOffset<MESH_TYPE>(out_slots);
  }
  MESH_TYPE* CreateMeshData() {
    return mesh_manager->CreateMeshData<MESH_TYPE>();
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
