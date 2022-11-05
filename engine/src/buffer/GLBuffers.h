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
  Arena instances_slots;

  inline MemorySlot Allocate(const unsigned int size) {
    return instances_slots.Allocate(size);
  };
  inline void Release(MemorySlot& slot) {
    instances_slots.Release(slot);
    slot = MemorySlots::NULL_SLOT;
  };
  inline void Release(MeshDataSlots& slot) {
    instances_slots.Release(slot.instances_slot);
    slot.instances_slot = MemorySlots::NULL_SLOT;
  };
  inline void Reset() {
    instances_slots.Reset();
  };
};

template<typename T>
struct BufferSlots {
  InstancesSlots instances_slots;
  // opengl generated buffer_id
  // dont confuse with DrawElementsIndirectCommand buffer_id (user provided)
  GLuint buffer_id;
  T* buffer_ptr;
  inline void Reset() {
    instances_slots.Reset();
  };
};

struct CommandBuffer {
  BufferSlots<DrawElementsIndirectCommand>* ptr;
  // we have to use lock here
  // because command buffer must be "unmapped" before each drawcall
  BufferLock* buffer_lock;
  inline bool operator==(const CommandBuffer& other) { return ptr->buffer_id == other.ptr->buffer_id; }
  ~CommandBuffer() {
    delete buffer_lock;
  }
};

template<typename T>
struct BufferNumberPool {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  // (or MAX_VERTEX, MAX_INDEX, ...)
  ThreadSafeNumberPool instances_slots;
  GLuint buffer_id;
  T* buffer_ptr;

  inline unsigned int Allocate() {
    return instances_slots.ObtainNumber();
  };
  inline void Release(unsigned int number) {
    instances_slots.ReleaseNumber(number);
  };
  inline void Reset() {
    instances_slots.Reset();
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
  BufferSlots<Vertex>* vertex_ptr;
  BufferSlots<unsigned int>* index_ptr;

  CommandBuffer* command_ptr;
  BufferSlots<LightsUniform>* light_ptr;
  BufferNumberPool<GLuint64>* texture_handle_by_texture_id_ptr;
  BufferSlots<UniformDataMesh>* mesh_uniform_ptr;
  BufferSlots<UniformDataOverlay3D>* uniforms_3d_overlay_ptr;
  BufferSlots<UniformDataUI>* uniforms_ui_ptr;
  BufferSlots<ControllerUniformData>* uniforms_controller_ptr;

  // call after SSBO write (GL_SHADER_STORAGE_BUFFER).
  void SetSyncBarrier() {
    is_need_barrier = true;
  }
  void InitBuffers();
  void BindVAOandBuffers();
  void Dispose();
  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void MapBuffer();
  void PreDraw();
  void PostDraw();
  void Reset() {
    fence_sync = 0;
    used_buffers = 0;

    vertex_ptr->Reset();
    index_ptr->Reset();
    command_ptr->ptr->Reset();
    light_ptr->Reset();
    texture_handle_by_texture_id_ptr->Reset();
    mesh_uniform_ptr->Reset();
    uniforms_3d_overlay_ptr->Reset();
    uniforms_ui_ptr->Reset();
    uniforms_controller_ptr->Reset();
  };

  ///////////////GetBufferSlots template//////////////////////////////////
  /**
   * per shader list of buffer_id->transform/texture array offset
   * thus, have to get pointer to structure for each shader separately
   * cannot have shared buffer_id->... array, because buffer_id is per draw call
   *                                      (buffer_id is not shared intentionally)
   *
  */
  template<typename UNIFORM_DATA_TYPE, typename MESH_TYPE>
  BufferSlots<UNIFORM_DATA_TYPE>* GetBufferSlots() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline BufferSlots<UniformDataMesh>* GetBufferSlots<UniformDataMesh, MeshData>()
  {
    return mesh_uniform_ptr;
  }
  template<>
  inline BufferSlots<UniformDataUI>* GetBufferSlots<UniformDataUI, MeshDataUI>()
  {
    return uniforms_ui_ptr;
  }
  template<>
  inline BufferSlots<UniformDataOverlay3D>* GetBufferSlots<UniformDataOverlay3D, MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr;
  }
  /////////////////////////////////////////////////

  template<typename MESH_TYPE>
  unsigned int* GetBufferSlotsBaseInstanceOffset() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline unsigned int* GetBufferSlotsBaseInstanceOffset<MeshData>()
  {
    return mesh_uniform_ptr->buffer_ptr->base_instance_offset;
  };
  template<>
  inline unsigned int* GetBufferSlotsBaseInstanceOffset<MeshDataUI>()
  {
    return uniforms_ui_ptr->buffer_ptr->base_instance_offset;
  };
  template<>
  inline unsigned int* GetBufferSlotsBaseInstanceOffset<MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr->buffer_ptr->base_instance_offset;
  };
  /////////////////////////////////////////////////
  template<typename MESH_TYPE>
  InstancesSlots& GetInstancesSlot() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline InstancesSlots& GetInstancesSlot<MeshData>()
  {
    return mesh_uniform_ptr->instances_slots;
  }
  template<>
  inline InstancesSlots& GetInstancesSlot<MeshDataUI>()
  {
    return uniforms_ui_ptr->instances_slots;
  }
  template<>
  inline InstancesSlots& GetInstancesSlot<MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr->instances_slots;
  }
  /////////////////////////////////////////////////

  //////////////GetTransformPtr template///////////
  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  TRANSFORM_TYPE* GetTransformPtr() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline mat4* GetTransformPtr<mat4, MeshData>()
  {
    return mesh_uniform_ptr->buffer_ptr->transforms;
  }
  template<>
  inline vec2* GetTransformPtr<vec2, MeshDataUI>()
  {
    return uniforms_ui_ptr->buffer_ptr->transforms;
  }
  template<>
  inline mat4* GetTransformPtr<mat4, MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr->buffer_ptr->transforms;
  }
  /////////////////////////////////////////////////

  //////////////GetNumCommands template///////////
  template<typename MESH_TYPE>
  inline unsigned int GetNumCommands();

  template<>
  inline unsigned int GetNumCommands<MeshData>()
  {
    return mesh_uniform_ptr->instances_slots.instances_slots.GetUsed();
  }
  template<>
  inline unsigned int GetNumCommands<MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr->instances_slots.instances_slots.GetUsed();
  }
  template<>
  inline unsigned int GetNumCommands<MeshDataUI>()
  {
    return uniforms_ui_ptr->instances_slots.instances_slots.GetUsed();
  }
  /////////////////////////////////////////////////

  //////////////BufferBlendTextures template///////////
  template<typename MESH_TYPE, typename TEXTURE_ARRAY_TYPE>
  void BufferTexture(BufferInstanceOffset& mesh, TEXTURE_ARRAY_TYPE& texture) {
    throw runtime_error("Not implemented");
  }
  template<>
  inline void BufferTexture<MeshData, BlendTextures>(BufferInstanceOffset& mesh, BlendTextures& textures) {
    mesh_uniform_ptr->buffer_ptr->blend_textures[mesh.GetInstanceOffset()]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataUI, unsigned int>(BufferInstanceOffset& mesh, unsigned int& texture_id) {
    uniforms_ui_ptr->buffer_ptr->texture_id_by_instance_id[mesh.GetInstanceOffset()]
      = texture_id;
  };
  // TODO: 
  template<>
  inline void BufferTexture<MeshDataOverlay3D, unsigned int>(BufferInstanceOffset& mesh, unsigned int& texture_id) {
    throw runtime_error("Not implemented");
  };
  /////////////////////////////////////////////////

  inline MemorySlot AllocateCommandBufferSlot(unsigned int size) {
    return command_ptr->ptr->instances_slots.Allocate(size);
  }
  inline void ReleaseCommandBufferSlot(MemorySlot& out_slot) {
    command_ptr->ptr->instances_slots.Release(out_slot);
  }
private:
  void _SyncGPUBufAndUnmap();
  void _BindCommandBuffer();
  void _UnmapBuffer();
  void _DeleteCommandBuffer(CommandBuffer* command_ptr) {
    lock_guard<mutex> data_lock(command_ptr->buffer_lock->data_mutex);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, command_ptr->ptr->buffer_id);
    glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
    DEBUG_EXPR(CheckGLError());
    delete command_ptr;
  }
  void WaitGPU(GLsync fence_sync,
               const source_location& location = source_location::current());
  template<typename T>
  BufferSlots<T>* _CreateBufferStorageSlots(unsigned long num_elements,
    GLuint array_type,
    bool is_allocate_powers_of_2 = false);
  template<typename T>
  BufferNumberPool<T>* _CreateBufferStorageNumberPool(unsigned long storage_size,
    GLuint array_type);
  CommandBuffer* _CreateCommandBuffer();
};
