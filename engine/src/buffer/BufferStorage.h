#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Settings.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"
#include "OpenGLHelpers.h"
#include "Logging.h"
#include "GPUUniformsStruct.h"
#include "AnimationStruct.h"
#include "LightStruct.h"
#include "GLBuffers.h"
#include "TextureStruct.h"
#include "Texture.h"
#include "MeshDataStruct.h"
#include "OverlayStruct.h"
#include "Vertex.h"
#include "GPUSynchronizer.h"

/**
Only command buffer must be contigious
(to issue drawcall with multiple commands they must be packed together)
Vertex/Index buffers take offsets from commands, can be allocated anywhere.
*/
using namespace std;
using namespace glm;
using namespace BufferSettings;
using namespace UniformsLocations;
using namespace BufferSizes;

class BufferStorage {

public:

  GLBuffers* gl_buffers;

  static BufferStorage* GetInstance() {
    static BufferStorage* instance = new BufferStorage();
    return instance;
  };

  void Reset() {
  };

  void Init();

  template<typename MESH_TYPE>
  bool AllocateInstanceSlot(
      MeshDataSlots& out_slots,
      unsigned long num_instances
  ) {
    unsigned int* base_instance_offset_ptr = gl_buffers->GetBufferSlotsBaseInstanceOffset<MESH_TYPE>();
    out_slots.instances_slot = gl_buffers->GetInstancesSlot<MESH_TYPE>().Allocate(num_instances);
    // because buffer id must be initialized before this call. (currently in DrawCall)
    assert(out_slots.IsBufferIdAllocated());
    if (out_slots.instances_slot == MemorySlots::NULL_SLOT) {
      LOG("Error _AllocateInstanceSlot.");
      return false;
    }
    base_instance_offset_ptr[out_slots.buffer_id] = out_slots.instances_slot.offset;
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
    return true;
  };

  /**
   * reallocate instance slot if new_instance_count doesnt fit existing slot
   * --- @return if slot changed
   * +++ @return if slot allocated successfully
  */
  template<typename MESH_TYPE>
  bool TryReallocateInstanceSlot(MeshDataSlots& mesh_slots,
    GLuint& new_instance_count)
  {
    if (new_instance_count == 0) {
      ReleaseInstanceSlot<MESH_TYPE>(mesh_slots);
      return true;
    }
    else if (new_instance_count <= mesh_slots.instances_slot.count) {
      // existing slot already fits requested amount
      return true;
    }
    else if (mesh_slots.instances_slot != MemorySlots::NULL_SLOT) {
      ReleaseInstanceSlot<MESH_TYPE>(mesh_slots);
    }
    return AllocateInstanceSlot<MESH_TYPE>(mesh_slots, new_instance_count);
  }

  template<typename MESH_TYPE>
  inline void ReleaseInstanceSlot(MeshDataSlots& out_slots) {
    auto& instances_slots = gl_buffers->GetInstancesSlot<MESH_TYPE>();
    instances_slots.Release(out_slots.instances_slot);
  };
  void BufferBoneTransform(unordered_map<unsigned int, mat4>& id_to_transform);
  void BufferBoneTransform(Bone* bone, mat4& trans, unsigned int num_bones = 1);
  void BufferLights(vector<Light*>& lights);

  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  void BufferTransform(BufferInstanceOffset* mesh, TRANSFORM_TYPE& transform) {
    assert(mesh->IsInstanceOffsetAllocated());
    gl_buffers->GetTransformPtr<TRANSFORM_TYPE, MESH_TYPE>()[mesh->GetInstanceOffset()] = transform;
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  };
  template<typename MESH_TYPE>
  unsigned int GetNumCommands() {
    return gl_buffers->GetNumCommands<MESH_TYPE>();
  };

  template<typename MESH_TYPE, typename TEXTURE_ARRAY_TYPE>
  void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    assert(mesh->IsInstanceOffsetAllocated());
    gl_buffers->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(mesh, texture);
  }

  /**
   * used by TextureManager, should not anywhere else
  */
  Texture* CreateTextureWithBufferSlot(GLuint gl_texture_id, GLuint64 gl_texture_handle_ARB);
  void ReleaseTexture(Texture* texture);
  void BufferUniformDataUISize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void BufferUniformDataController(int mouse_x, int mouse_y, int is_pressed, int is_click);

  void AddUsedBuffers(BufferType::BufferTypeFlag used_buffers);
  void BindBuffers();
private:
  void _BufferTextureHandle(Texture* texture);
  BufferStorage() :
    gl_buffers{ new GLBuffers() }{};
  ~BufferStorage() {
    delete gl_buffers;
  };
};
