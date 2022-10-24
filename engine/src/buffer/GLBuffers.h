#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "GPUUniformsStruct.h"
#include "MeshDataStruct.h"
#include "Vertex.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"

using namespace std;
using namespace UniformsLocations;
using namespace BufferSizes;

struct InstancesSlots {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  // (or MAX_VERTEX, MAX_INDEX, ...)
  Arena* instances_slots;

  inline MemorySlot Allocate(const unsigned int size) {
    auto slot = instances_slots->Allocate(size);
  };
  inline void Release(MemorySlot& slot) {
    instances_slots->Release(slot);
    slot = MemorySlots::NULL_SLOT;
  };
  inline void Release(MeshDataSlots& slot) {
    instances_slots->Release(slot.instances_slot);
    slot.instances_slot = MemorySlots::NULL_SLOT;
  };
  inline void Reset() {
    instances_slots->Reset();
  };
};

template<typename T>
struct BufferSlots {
  InstancesSlots instances_slots;
  GLuint buffer_id;
  T* buffer_ptr;
  inline void Reset() {
    instances_slots.Reset();
  };
};

template<typename T>
struct BufferNumberPool {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  // (or MAX_VERTEX, MAX_INDEX, ...)
  ThreadSafeNumberPool* instances_slots;
  GLuint buffer_id;
  T* buffer_ptr;

  inline unsigned int Allocate() {
    auto slot = instances_slots->ObtainNumber();
  };
  inline unsigned int Release(unsigned int number) {
    instances_slots->ReleaseNumber(number);
  };
  inline void Reset() {
    instances_slots->Reset();
  };
};

class GLBuffers {
  ///////////
  /// Buffers
  ///////////
  // currently all shaders/commands
  // are using shared vertex/index array,
  // associated with this VAO
  GLuint mesh_VAO;

  BufferType::BufferTypeFlag used_buffers;
  BufferType::BufferTypeFlag bound_buffers;

  const GLbitfield flags =
    GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
  GLsync fence_sync = 0;
  bool is_need_barrier = false;
public:
  //////////////////////////
  // Mapped buffers pointers
  //////////////////////////
  BufferSlots<Vertex> vertex_ptr;
  BufferSlots<unsigned int> index_ptr;

  BufferSlots<CommandBuffer> command_ptr;
  //BlendTexturesMeshUniform* blend_textures_mesh_ptr;
  BufferSlots<LightsUniform> light_ptr;
  BufferNumberPool<GLuint64> texture_handle_by_texture_id_ptr;
  BufferSlots<UniformDataMesh> mesh_uniform_ptr;
  BufferSlots<UniformDataOverlay3D> uniforms_3d_overlay_ptr;
  BufferSlots<UniformDataUI> uniforms_ui_ptr;
  BufferSlots<ControllerUniformData> uniforms_controller_ptr;

  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  void SetSyncBarrier() {
    is_need_barrier = true;
  }
  void Reset() {
    fence_sync = 0;
    used_buffers = 0;
  };
  void InitBuffers();
  void BindVAOandBuffers();
  void Dispose();
  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void MapBuffer();
  void PreDraw();
  void PostDraw();
  void Reset() {
    vertex_ptr.Reset();
    index_ptr.Reset();
    command_ptr.Reset();
    light_ptr.Reset();
    texture_handle_by_texture_id_ptr.Reset();
    mesh_uniform_ptr.Reset();
    uniforms_3d_overlay_ptr.Reset();
    uniforms_ui_ptr.Reset();
    uniforms_controller_ptr.Reset();
  };

  ///////////////GetBufferSlots template//////////////////////////////////
  /**
   * per shader list of buffer_id->transform/texture array offset
   * thus, have to get pointer to structure for each shader separately
   * cannot have shared buffer_id->... array, because buffer_id is per draw call
   *                                      (buffer_id is not shared intentionally)
   *
  */
  template<typename UNIFORM_TYPE, typename MESH_DATA_TYPE>
  BufferSlots<UNIFORM_TYPE>& GetBufferSlots(MESH_DATA_TYPE& mesh);
  inline BufferSlots<UniformDataMesh>& GetBufferSlots(MeshData& mesh)
  {
    return mesh_uniform_ptr;
  }
  inline BufferSlots<UniformDataUI>& GetBufferSlots(MeshDataUI& mesh)
  {
    return uniforms_ui_ptr;
  }
  inline BufferSlots<UniformDataOverlay3D>& GetBufferSlots(MeshDataOverlay3D& mesh)
  {
    return uniforms_3d_overlay_ptr;
  }
  /////////////////////////////////////////////////

  //////////////GetTransformPtr template///////////
  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  TRANSFORM_TYPE* GetTransformPtr(MESH_TYPE& mesh);
  
  inline mat4* GetTransformPtr(MeshData& mesh)
  {
    return mesh_uniform_ptr.buffer_ptr->transforms;
  }
  inline vec2* GetTransformPtr(MeshDataUI& mesh)
  {
    return uniforms_ui_ptr.buffer_ptr->transforms;
  }
  inline mat4* GetTransformPtr(MeshDataOverlay3D& mesh)
  {
    return uniforms_3d_overlay_ptr.buffer_ptr->transforms;
  }
  /////////////////////////////////////////////////

private:
  void _SyncGPUBufAndUnmap();
  void _BindCommandBuffer();
  void _UnmapBuffer();
  BufferSlots<CommandBuffer> _CreateCommandBuffer();
  void _DeleteCommandBuffer(CommandBuffer* command_ptr) {
    lock_guard<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_ptr->id);
    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    DEBUG_EXPR(CheckGLError());
    delete command_ptr;
  }
  void WaitGPU(GLsync fence_sync,
               const source_location& location = source_location::current());
  template<typename T>
  BufferSlots<T> _CreateBufferStorageSlots(unsigned long num_elements);
  template<typename T>
  BufferNumberPool<T> _CreateBufferStorageNumberPool(unsigned long storage_size);
};
