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

/**
Only command buffer must be contigious
(to issue drawcall with multiple commands they must be packed together)
Vertex/Index buffers take offsets from commands, can be allocated anywhere.

TODO:
Slot release logic on deleted BaseMesh/Command/Vertex/Index etc.
maybe predefined slot types of same size, to avoid fragmentation

try coroutines instead of locks
*/
using namespace std;
using namespace glm;
using namespace BufferSettings;
using namespace UniformsLocations;
using namespace BufferSizes;

class BufferStorage {

private:
  //  TODO: each drawcall uses contigious range of commands. Need to allocate in advance for shader.
  //    or place shader with dynamic number of meshes at the end

public:
  GLBuffers* gl_buffers;

  static BufferStorage* GetInstance() {
    static BufferStorage* instance = new BufferStorage();
    return instance;
  };

  void Reset() {
    gl_buffers->Reset();
  };

  void Init();

  void BufferCommands(vector<DrawElementsIndirectCommand>& active_commands, int command_offset);
  void BufferCommand(DrawElementsIndirectCommand& command, int command_offset);
  void BufferMeshData(MeshDataSlots& slots, shared_ptr<MeshLoadData>& load_data);
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size);
  template<typename MESH_TYPE>
  bool AllocateInstanceSlot(
      MeshDataSlots& out_slots,
      unsigned long num_instances
  ) {
    unsigned int* base_instance_offset_ptr = gl_buffers->GetBufferSlotsBaseInstanceOffset<MESH_TYPE>();
    InstancesSlots& instances_slot = gl_buffers->GetInstancesSlot<MESH_TYPE>();
    // because buffer id must be initialized before this call. (currently in DrawCall)
    DEBUG_ASSERT(out_slots.buffer_id >= 0);
    out_slots.instances_slot = instances_slot.Allocate(num_instances);
    if (out_slots.instances_slot == MemorySlots::NULL_SLOT) {
      LOG("Error _AllocateInstanceSlot.");
      return false;
    }
    base_instance_offset_ptr[out_slots.buffer_id] = out_slots.instances_slot.offset;
    gl_buffers->SetSyncBarrier();
    return true;
  };
  void ReleaseStorage(MeshDataSlots& out_slots);
  template<typename MESH_TYPE>
  inline void ReleaseInstanceSlot(MeshDataSlots& out_slots) {
    auto& instances_slots = gl_buffers->GetInstancesSlot<MESH_TYPE>();
    instances_slots.Release(out_slots.instances_slot);
  };
  inline MemorySlot AllocateCommandBufferSlot(unsigned int size) {
    return gl_buffers->AllocateCommandBufferSlot(size);
  }

  void BufferBoneTransform(unordered_map<unsigned int, mat4>& id_to_transform);
  void BufferBoneTransform(Bone* bone, mat4& trans, unsigned int num_bones = 1);
  void BufferLights(vector<Light*>& lights);

  template<typename TRANSFORM_TYPE, typename MESH_TYPE>
  inline void BufferTransform(BufferInstanceOffset& mesh, TRANSFORM_TYPE& transform) {
    gl_buffers->GetTransformPtr<TRANSFORM_TYPE, MESH_TYPE>()[mesh.GetInstanceOffset()] = transform;
    gl_buffers->SetSyncBarrier();
  };
  template<typename MESH_TYPE>
  inline unsigned int GetNumCommands() {
    return gl_buffers->GetNumCommands<MESH_TYPE>();
  };

  template<typename MESH_TYPE, typename TEXTURE_ARRAY_TYPE>
  inline void BufferTexture(BufferInstanceOffset& mesh, TEXTURE_ARRAY_TYPE& texture) {
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

  void PreDraw();
  void PostDraw();
private:
  void _BufferTextureHandle(Texture* texture);
  BufferStorage() :
    gl_buffers{ new GLBuffers() }{};
  ~BufferStorage() {
    delete gl_buffers;
  };
};
