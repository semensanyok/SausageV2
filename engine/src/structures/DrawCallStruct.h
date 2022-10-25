#pragma once

#include "sausage.h"
#include "ShaderStruct.h"
#include "Arena.h"
#include "GPUStructs.h"
#include "MeshDataStruct.h"

using namespace std;

enum DrawOrder {
  MESH,
  OVERLAY_3D,
  PHYS_DEBUG,
  UI_BACK,
  UI_TEXT,
};

// MeshDataClass is used to determine which buffer to use to allocate instance_id for shader
// (referenced from base_instance_offset[buffer_id] -> instance_id to fetch textures/transforms)
template<typename MeshDataClass>
class DrawCall {
  friend class DrawCallManager;
  // now each draw call has its own buffer offset array in its uniform
  // see GlBuffers->
  //MemorySlot command_buffer_slot;
  //Arena* command_buffer_sub_arena;
  ThreadSafeNumberPool* buffer_id_slots;
public:
  bool is_enabled;
  /**
   * @param is_reserve_command_count_in_buffer
   *            set this flag when expect
   *          to add more DrawElementsIndirectCommand
   *          to this draw_call.
   *            false when not expect to change.
   *          i.e. single instanced draw call, where only instance count changes.
   */
  DrawCall(unsigned int id,
    Shader* shader,
    GLenum mode,
    bool is_enabled) :
    id{ id },
    shader{ shader },
    mode{ mode },
    // here we have offsets to shared command array,
    // used by glDrawElementsIndirect,
    // with range (offset, offset + count)
    // and its buffer_id array with range (0, count)
    buffer_id_slots { new ThreadSafeNumberPool(command_buffer_slot.count) },
    is_enabled{ is_enabled } {
  }
  const unsigned int id;
  // caller must aquire it on write
  mutex mtx;
  GLenum mode;  // GL_TRIANGLES GL_LINES
  Shader* shader;
  unsigned int GetCommandCount() {
    return command_buffer_sub_arena->GetUsed();
  }
  unsigned int GetBaseOffset() {
    return command_buffer_sub_arena->GetBaseOffset();
  }
private:
  void Allocate(MeshDataSlots& out_slots, unsigned int instances_count) {
    out_slots.buffer_id = buffer_id_slots->ObtainNumber();
    DEBUG_ASSERT(out_slots.buffer_id >= 0);
    out_slots.instances_slot = 
    return slot;
  }
  void Release(MemorySlot slot) {
    command_buffer_sub_arena->Release(slot);
  }
};
