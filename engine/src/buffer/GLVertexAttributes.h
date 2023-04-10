#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "Vertex.h"
#include "GLBuffersCommon.h"
#include "GPUSynchronizer.h"
#include "GPUUniformsStruct.h"
#include "Logging.h"
#include "MeshDataStruct.h"

using namespace std;
using namespace BufferSizes;

class GLVertexAttributes {

  VertexType bound_array = VertexType::NONE;

  GLuint mesh_VAO;
  GLuint static_VAO;
  GLuint ui_VAO;
  GLuint outline_VAO;

  //////////////////////////
  // Mapped buffers pointers
  //////////////////////////
  BufferSlots<Vertex>* vertex_ptr;
  BufferSlots<VertexStatic>* vertex_static_ptr;
  BufferSlots<VertexUI>* vertex_ui_ptr;
  BufferSlots<VertexOutline>* vertex_outline_ptr;

  BufferSlots<unsigned int>* index_ptr;
  // need ?????????
  BufferSlots<unsigned int>* index_static_ptr;
  BufferSlots<unsigned int>* index_ui_ptr;
  BufferSlots<unsigned int>* index_outline_ptr;


public:
  void InitVAO() {
    glGenVertexArrays(1, &mesh_VAO);
    glBindVertexArray(mesh_VAO);
    vertex_ptr = CreateBufferSlots<Vertex>(VERTEX_STORAGE_SIZE, MAX_VERTEX, GL_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    index_ptr = CreateBufferSlots<unsigned int>(INDEX_STORAGE_SIZE, MAX_INDEX, GL_ELEMENT_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ptr->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_ptr->buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, Bitangent));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex),
      (void*)offsetof(Vertex, BoneIds));
    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
      (void*)offsetof(Vertex, BoneWeights));

    glGenVertexArrays(1, &static_VAO);
    glBindVertexArray(static_VAO);
    vertex_static_ptr = CreateBufferSlots<VertexStatic>(VERTEX_STATIC_STORAGE_SIZE, MAX_VERTEX_STATIC, GL_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    index_static_ptr = CreateBufferSlots<unsigned int>(INDEX_STATIC_STORAGE_SIZE, MAX_INDEX_STATIC, GL_ELEMENT_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_static_ptr->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_static_ptr->buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic),
      (void*)offsetof(VertexStatic, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexStatic),
      (void*)offsetof(VertexStatic, TexCoords));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic),
      (void*)offsetof(VertexStatic, Tangent));
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(VertexStatic),
      (void*)offsetof(VertexStatic, Bitangent));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, sizeof(VertexStatic),
      (void*)offsetof(VertexStatic, UniformId));

    glGenVertexArrays(1, &ui_VAO);
    glBindVertexArray(ui_VAO);
    vertex_ui_ptr = CreateBufferSlots<VertexUI>(VERTEX_UI_STORAGE_SIZE, MAX_VERTEX_UI, GL_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    index_ui_ptr = CreateBufferSlots<unsigned int>(INDEX_UI_STORAGE_SIZE, MAX_INDEX_UI, GL_ELEMENT_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ui_ptr->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_ui_ptr->buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexUI), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexUI),
      (void*)offsetof(VertexUI, Normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(VertexUI),
      (void*)offsetof(VertexUI, TexCoords));

    glGenVertexArrays(1, &outline_VAO);
    glBindVertexArray(outline_VAO);
    vertex_outline_ptr = CreateBufferSlots<VertexOutline>(VERTEX_OUTLINE_STORAGE_SIZE, MAX_VERTEX_OUTLINE, GL_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    index_outline_ptr = CreateBufferSlots<unsigned int>(INDEX_OUTLINE_STORAGE_SIZE, MAX_INDEX_OUTLINE, GL_ELEMENT_ARRAY_BUFFER, ArenaSlotSize::POWER_OF_TWO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_outline_ptr->buffer_id);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_outline_ptr->buffer_id);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOutline), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(VertexOutline),
      (void*)offsetof(VertexOutline, Normal));

    DEBUG_EXPR(CheckGLError());
  };

  /**
   * MUST be bound before glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_buffer)
  */
  void BindVAO(VertexType vertex_type) {
    if (bound_array == vertex_type) {
      return;
    }
    switch (vertex_type) {
    case VertexType::MESH:
      BindMeshVAO();
      break;
    case VertexType::STATIC:
      BindStaticVAO();
      break;
    case VertexType::UI:
      BindUiVAO();
      break;
    case VertexType::OUTLINE:
      BindOutlineVAO();
      break;
    default:
      LOG(std::format("Error. Unknown vertex type"));
      return;
    };
    bound_array = vertex_type;
  }
  template <typename VERTEX_TYPE>
  void ReleaseStorage(MeshDataSlots& out_slots);
  template <typename VERTEX_TYPE>
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size
  ) {
    auto vertex_ptr = GetVertexPtr<VERTEX_TYPE>();
    out_slots.vertex_slot = vertex_ptr->instances_slots.Allocate(vertices_size);
    if (out_slots.vertex_slot == MemorySlots::NULL_SLOT) {
      LOG("Error RequestStorageSetOffsets vertices slot allocation.");
      return false;
    }
    auto index_ptr = GetIndexPtr<VERTEX_TYPE>();
    out_slots.index_slot = index_ptr->instances_slots.Allocate(indices_size);
    if (out_slots.index_slot == MemorySlots::NULL_SLOT) {
      vertex_ptr->instances_slots.Release(out_slots.vertex_slot);
      LOG("Error RequestStorageSetOffsets indices slot allocation.");
      return false;
    }
    return true;
  };
  template <typename VERTEX_TYPE>
  void BufferVertices(MeshDataSlots& slots,
                      shared_ptr<MeshLoadData<VERTEX_TYPE>>& load_data) {
    auto mesh_data = load_data.get();
    // storage must be allocated at this point (via AllocateStorage)
    assert("AllocateStorage was not called", slots.index_slot.count > 0);
    assert("AllocateStorage was not called", slots.vertex_slot.count > 0);
    assert("AllocateStorage was not called", slots.index_slot.count >= mesh_data->indices.size());
    assert("AllocateStorage was not called", slots.vertex_slot.count >= mesh_data->vertices.size());

    auto& vertices = mesh_data->vertices;
    auto& indices = mesh_data->indices;

    BufferVerticesIndices<VERTEX_TYPE>(slots, vertices, indices);
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  }
  void Dispose() {
    GPUSynchronizer::GetInstance()->SyncGPU();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(6);

    glDeleteVertexArrays(1, &mesh_VAO);
    glDeleteVertexArrays(1, &static_VAO);
    glDeleteVertexArrays(1, &ui_VAO);
    glDeleteVertexArrays(1, &outline_VAO);
    DEBUG_EXPR(CheckGLError());

    DeleteBuffer(GL_ARRAY_BUFFER, vertex_ptr->buffer_id);
    DeleteBuffer(GL_ARRAY_BUFFER, vertex_static_ptr->buffer_id);
    DeleteBuffer(GL_ARRAY_BUFFER, vertex_ui_ptr->buffer_id);
    DeleteBuffer(GL_ARRAY_BUFFER, vertex_outline_ptr->buffer_id);

    DeleteBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ptr->buffer_id);
    DeleteBuffer(GL_ELEMENT_ARRAY_BUFFER, index_static_ptr->buffer_id);
    DeleteBuffer(GL_ELEMENT_ARRAY_BUFFER, index_ui_ptr->buffer_id);
    DeleteBuffer(GL_ELEMENT_ARRAY_BUFFER, index_outline_ptr->buffer_id);
  }
  void Reset() {
    vertex_ptr->Reset();
    vertex_static_ptr->Reset();
    vertex_outline_ptr->Reset();

    index_ptr->Reset();
    index_static_ptr->Reset();
    index_ui_ptr->Reset();
    index_outline_ptr->Reset();
  }

  //////////////////////////////////////////////////////////////////////////////////////////////
  template<>
  void ReleaseStorage<Vertex>(MeshDataSlots& out_slots) {
    vertex_ptr->instances_slots.Release(out_slots.vertex_slot);
    index_ptr->instances_slots.Release(out_slots.index_slot);
  };
  template<>
  void ReleaseStorage<VertexStatic>(MeshDataSlots& out_slots) {
    vertex_static_ptr->instances_slots.Release(out_slots.vertex_slot);
    index_static_ptr->instances_slots.Release(out_slots.index_slot);
  };
  template<>
  void ReleaseStorage<VertexOutline>(MeshDataSlots& out_slots) {
    vertex_outline_ptr->instances_slots.Release(out_slots.vertex_slot);
    index_outline_ptr->instances_slots.Release(out_slots.index_slot);
  };
  //////////////////////////////////////////////////////////////////////////////////////////////
private:
  //////////////////////////////////////////////////////////////////////////////////////////////
  template <typename VERTEX_TYPE>
  void BufferVerticesIndices(MeshDataSlots& slots,
                             vector<VERTEX_TYPE>& vertices,
                             vector<unsigned int>& indices) {
    //int vertex_size = sizeof(Vertex);
    int vertex_size = sizeof(VERTEX_TYPE);
    auto vertex_ptr = GetVertexPtr<VERTEX_TYPE>();
    auto index_ptr = GetIndexPtr<VERTEX_TYPE>();

    memcpy(&vertex_ptr->buffer_ptr[slots.vertex_slot.offset], vertices.data(),
         vertices.size() * vertex_size);
    memcpy(&index_ptr->buffer_ptr[slots.index_slot.offset], indices.data(),
           indices.size() * sizeof(unsigned int));
  };

  //////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////////////////////
  template<typename VERTEX_TYPE>
  BufferSlots<VERTEX_TYPE>* GetVertexPtr();
  template<typename VERTEX_TYPE>
  BufferSlots<unsigned int>* GetIndexPtr();

  template<>
  BufferSlots<Vertex>* GetVertexPtr() { return vertex_ptr; };
  template<>
  BufferSlots<VertexStatic>* GetVertexPtr() { return vertex_static_ptr; };
  template<>
  BufferSlots<VertexUI>* GetVertexPtr() { return vertex_ui_ptr; };
  template<>
  BufferSlots<VertexOutline>* GetVertexPtr() { return vertex_outline_ptr; };
  template<>
  BufferSlots<unsigned int>* GetIndexPtr<Vertex>() { return index_ptr; };
  template<>
  BufferSlots<unsigned int>* GetIndexPtr<VertexStatic>() { return index_static_ptr; };
  template<>
  BufferSlots<unsigned int>* GetIndexPtr<VertexOutline>() { return index_outline_ptr; };
  template<>
  BufferSlots<unsigned int>* GetIndexPtr<VertexUI>() { return index_ui_ptr; };
  //////////////////////////////////////////////////////////////////////////////////////////////

  void BindOutlineVAO()
  {
    glBindVertexArray(outline_VAO);
  }
  void BindUiVAO()
  {
    glBindVertexArray(ui_VAO);
  }
  void BindStaticVAO()
  {
    glBindVertexArray(static_VAO);
  }
  void BindMeshVAO()
  {
    glBindVertexArray(mesh_VAO);
  }
};
