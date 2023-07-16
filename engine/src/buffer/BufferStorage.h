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
#include "InstanceSlot.h"
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

  inline static BufferStorage* GetInstance() {
    static BufferStorage* instance = new BufferStorage();
    return instance;
  };

  void Reset() {
  };

  void Init();


  template<typename UNIFORM_DATA_TYPE, typename MESH_TYPE>
  inline void ReleaseUniformOffset(BufferInstanceOffset* out_slots) {
    if (out_slots->IsUniformOffsetAllocated()) {
      auto instances_slots = gl_buffers->GetUniformSlots<UNIFORM_DATA_TYPE, MESH_TYPE>();
      instances_slots->Release(out_slots->GetUniformOffset());
    }
  };
  template<typename UNIFORM_DATA_TYPE, typename MESH_TYPE>
  bool AllocateUniformOffset(
      InstanceSlotUpdate* out_slots
  ) {
    auto uniform_offset = gl_buffers->GetUniformSlots<UNIFORM_DATA_TYPE, MESH_TYPE>()->Allocate();
    out_slots->SetUniformOffset(uniform_offset);
    return true;
  };

  template<typename MESH_TYPE>
  void BufferUniformOffset(
      BufferInstanceOffset* inst_slots
  ) {
    unsigned int* base_instance_offset_ptr = gl_buffers->GetUniformOffsetByInstanceId<MESH_TYPE>();
    // because buffer id must be initialized before this call. (currently in DrawCall)
    assert(inst_slots->IsInstanceOffsetAllocated());
    assert(inst_slots->IsUniformOffsetAllocated());
    base_instance_offset_ptr[inst_slots->GetInstanceOffset()] = inst_slots->GetUniformOffset();
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  };

  void BufferBoneTransform(unordered_map<unsigned int, mat4>& id_to_transform);
  void BufferBoneTransform(Bone* bone, mat4& trans, unsigned int num_bones = 1);
  void BufferLights(vector<Light*>& lights);

  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  void BufferTransform(BufferInstanceOffset* mesh, const TRANSFORM_TYPE& transform) {
    assert(mesh->IsUniformOffsetAllocated());
    auto offset = mesh->GetUniformOffset();
    gl_buffers->GetTransformPtr<TRANSFORM_TYPE, MESH_TYPE>()[offset] = transform;
    GPUSynchronizer::GetInstance()->SetSyncBarrier();
  };

  template<typename MESH_TYPE, typename TEXTURE_ARRAY_TYPE>
  void BufferTexture(BufferInstanceOffset* mesh, TEXTURE_ARRAY_TYPE& texture) {
    assert(mesh->IsUniformOffsetAllocated());
    gl_buffers->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(mesh, texture);
  }

  /**
   * used by TextureManager, should not anywhere else
  */
  Texture* CreateTextureWithBufferSlot(GLuint gl_texture_id, GLuint64 gl_texture_handle_ARB);
  void ReleaseTexture(Texture* texture);
  void BufferUniformDataUISize(BufferInstanceOffset* mesh, int min_x, int max_x, int min_y, int max_y);
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
