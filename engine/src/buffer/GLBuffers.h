#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "GPUUniformsStruct.h"
#include "MeshDataStruct.h"
#include "Vertex.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"
#include "GLBuffersCommon.h"
#include "GPUSynchronizer.h"

using namespace std;
using namespace UniformsLocations;
using namespace BufferSizes;

class GLBuffers {
    
  BufferType::BufferTypeFlag used_buffers = 0;
  BufferType::BufferTypeFlag bound_buffers = 0;

public:
  
  BufferSlots<LightsUniform>* light_ptr;
  BufferNumberPool<GLuint64>* texture_handle_by_texture_id_ptr;
  BufferSlots<UniformDataMesh>* mesh_uniform_ptr;
  BufferSlots<UniformDataOverlay3D>* uniforms_3d_overlay_ptr;
  BufferSlots<UniformDataUI>* uniforms_ui_ptr;
  BufferSlots<ControllerUniformData>* uniforms_controller_ptr;
  BufferSlots<UniformDataMeshStatic>* mesh_static_uniform_ptr;

  void InitBuffers();
  void BindBuffers();
  void Dispose();
  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void Reset() {
    used_buffers = 0;

    light_ptr->Reset();
    texture_handle_by_texture_id_ptr->Reset();
    mesh_uniform_ptr->Reset();
    uniforms_3d_overlay_ptr->Reset();
    uniforms_ui_ptr->Reset();
    uniforms_controller_ptr->Reset();
    mesh_static_uniform_ptr->Reset();
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
  inline BufferSlots<UniformDataMeshStatic>* GetBufferSlots<UniformDataMeshStatic, MeshDataStatic>()
  {
    return mesh_static_uniform_ptr;
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
  inline unsigned int* GetBufferSlotsBaseInstanceOffset<MeshDataStatic>()
  {
    return mesh_static_uniform_ptr->buffer_ptr->base_instance_offset;
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
  inline InstancesSlots& GetInstancesSlot<MeshDataStatic>()
  {
    return mesh_static_uniform_ptr->instances_slots;
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
  inline mat4* GetTransformPtr<mat4, MeshDataStatic>()
  {
    return mesh_static_uniform_ptr->buffer_ptr->transforms;
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
  inline unsigned int GetNumCommands<MeshDataStatic>()
  {
    return mesh_static_uniform_ptr->instances_slots.instances_slots.GetUsed();
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
  void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    throw runtime_error("Not implemented");
  }
  template<>
  inline void BufferTexture<MeshData, BlendTextures>(BufferInstanceOffset* mesh, BlendTextures& textures) {
    mesh_uniform_ptr->buffer_ptr->blend_textures[mesh->GetInstanceOffset()]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataStatic, BlendTextures>(BufferInstanceOffset* mesh, BlendTextures& textures) {
    mesh_static_uniform_ptr->buffer_ptr->blend_textures[mesh->GetInstanceOffset()]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataUI, unsigned int>(BufferInstanceOffset* mesh, unsigned int& texture_id) {
    uniforms_ui_ptr->buffer_ptr->texture_id_by_instance_id[mesh->GetInstanceOffset()]
      = texture_id;
  };
  // TODO: 
  template<>
  inline void BufferTexture<MeshDataOverlay3D, unsigned int>(BufferInstanceOffset* mesh, unsigned int& texture_id) {
    throw runtime_error("Not implemented");
  };
};
