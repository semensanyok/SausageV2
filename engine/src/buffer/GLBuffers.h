#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "GPUUniformsStruct.h"
#include "MeshDataStruct.h"
#include "TerrainStruct.h"
#include "Vertex.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"
#include "GLBuffersCommon.h"
#include "GPUSynchronizer.h"
#include "InstanceSlot.h"

using namespace std;
using namespace UniformsLocations;
using namespace BufferSizes;


class MeshData;
class MeshDataStatic;
class MeshDataTerrain;
class MeshDataUI;
class MeshDataOverlay3D;

class GLBuffers {
    
  BufferType::BufferTypeFlag used_buffers = 0;
  BufferType::BufferTypeFlag bound_buffers = 0;

public:
  
  BufferNumberPool<LightsUniform>* light_ptr;
  BufferNumberPool<GLuint64>* texture_handle_by_texture_id_ptr;
  BufferNumberPool<UniformDataMesh>* mesh_uniform_ptr;
  BufferNumberPool<UniformDataMeshStatic>* mesh_static_uniform_ptr;
  BufferNumberPool<UniformDataMeshTerrain>* mesh_terrain_uniform_ptr;
  BufferNumberPool<UniformDataOverlay3D>* uniforms_3d_overlay_ptr;
  BufferNumberPool<UniformDataUI>* uniforms_ui_ptr;
  BufferNumberPool<ControllerUniformData>* uniforms_controller_ptr;

  // terrain specific
  Arena* terrain_texture_instance_slots;

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
    mesh_terrain_uniform_ptr->Reset();
  };

  ///////////////GetUniformSlots template//////////////////////////////////
  /**
   * per shader list of buffer_id->transform/texture array offset
   * thus, have to get pointer to structure for each shader separately
   * cannot have shared buffer_id->... array, because buffer_id is per draw call
   *                                      (buffer_id is not shared intentionally)
   *
  */
  template<typename UNIFORM_DATA_TYPE, typename MESH_TYPE>
  BufferNumberPool<UNIFORM_DATA_TYPE>* GetUniformSlots() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline BufferNumberPool<UniformDataMesh>* GetUniformSlots<UniformDataMesh, MeshData>()
  {
    return mesh_uniform_ptr;
  }
  template<>
  inline BufferNumberPool<UniformDataMeshStatic>* GetUniformSlots<UniformDataMeshStatic, MeshDataStatic>()
  {
    return mesh_static_uniform_ptr;
  }
  template<>
  inline BufferNumberPool<UniformDataMeshTerrain>* GetUniformSlots<UniformDataMeshTerrain, MeshDataTerrain>()
  {
    return mesh_terrain_uniform_ptr;
  }
  template<>
  inline BufferNumberPool<UniformDataUI>* GetUniformSlots<UniformDataUI, MeshDataUI>()
  {
    return uniforms_ui_ptr;
  }
  template<>
  inline BufferNumberPool<UniformDataOverlay3D>* GetUniformSlots<UniformDataOverlay3D, MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr;
  }
  /////////////////////////////////////////////////

  template<typename MESH_TYPE>
  unsigned int* GetUniformOffsetByInstanceId() {
    throw runtime_error("Not implemented");
  };
  template<>
  inline unsigned int* GetUniformOffsetByInstanceId<MeshData>()
  {
    return mesh_uniform_ptr->buffer_ptr->uniform_offset;
  };
  template<>
  inline unsigned int* GetUniformOffsetByInstanceId<MeshDataStatic>()
  {
    return mesh_static_uniform_ptr->buffer_ptr->uniform_offset;
  };
  template<>
  inline unsigned int* GetUniformOffsetByInstanceId<MeshDataTerrain>()
  {
    return mesh_terrain_uniform_ptr->buffer_ptr->uniform_offset;
  };
  template<>
  inline unsigned int* GetUniformOffsetByInstanceId<MeshDataUI>()
  {
    return uniforms_ui_ptr->buffer_ptr->uniform_offset;
  };
  template<>
  inline unsigned int* GetUniformOffsetByInstanceId<MeshDataOverlay3D>()
  {
    return uniforms_3d_overlay_ptr->buffer_ptr->uniform_offset;
  };

  template<typename TEXTURE_TYPE>
  TEXTURE_TYPE* GetTexturesSlot() {
  }

  template<>
  inline TerrainBlendTextures* GetTexturesSlot<TerrainBlendTextures>()
  {
    auto slot = terrain_texture_instance_slots->Allocate(1);
    if (slot == MemorySlots::NULL_SLOT) {
      LOG("Error allocating slot for TerrainBlendTextures");
      return nullptr;
    }
    return new TerrainBlendTextures(slot.offset);
  }

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
  inline mat4* GetTransformPtr<mat4, MeshDataTerrain>()
  {
    return mesh_terrain_uniform_ptr->buffer_ptr->transforms;
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

  //////////////BufferBlendTextures template///////////
  template<typename MESH_TYPE, typename TEXTURE_ARRAY_TYPE>
  void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    throw runtime_error("Not implemented");
  }
  template<>
  inline void BufferTexture<MeshData, BlendTextures>(BufferInstanceOffset* mesh, BlendTextures& textures) {
    auto offset = mesh->GetUniformOffset();
    mesh_uniform_ptr->buffer_ptr->blend_textures[offset]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataStatic, BlendTextures>(BufferInstanceOffset* mesh, BlendTextures& textures) {
    auto offset = mesh->GetUniformOffset();
    mesh_static_uniform_ptr->buffer_ptr->blend_textures[offset]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataTerrain, BlendTextures>(BufferInstanceOffset* mesh, BlendTextures& textures) {
    auto offset = mesh->GetUniformOffset();
    mesh_terrain_uniform_ptr->buffer_ptr->blend_textures[offset]
      = textures;
  };
  template<>
  inline void BufferTexture<MeshDataUI, unsigned int>(BufferInstanceOffset* mesh, unsigned int& texture_id) {
    auto offset = mesh->GetUniformOffset();
    uniforms_ui_ptr->buffer_ptr->texture_id_by_instance_id[offset]
      = texture_id;
  };
  // TODO: 
  template<>
  inline void BufferTexture<MeshDataOverlay3D, unsigned int>(BufferInstanceOffset* mesh, unsigned int& texture_id) {
    throw runtime_error("Not implemented");
  };
};
