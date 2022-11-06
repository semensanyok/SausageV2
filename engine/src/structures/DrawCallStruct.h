#pragma once

#include "sausage.h"
#include "ShaderStruct.h"
#include "Arena.h"
#include "GPUStructs.h"
#include "MeshDataStruct.h"
#include "ThreadSafeNumberPool.h"
#include "Macros.h"

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
// commented out because now it is used only in template function
//template<typename MeshDataClass>
class DrawCall {
  friend class DrawCallManager;
  // now each draw call has its own buffer offset array in its uniform
  // see GlBuffers->
  MemorySlot command_buffer_slot;
  Arena command_buffer_sub_arena;
  ThreadSafeNumberPool buffer_id_slots;
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
    MemorySlot command_buffer_slot,
    bool is_enabled) :
    id{ id },
    shader{ shader },
    mode{ mode },
    command_buffer_slot{ command_buffer_slot },
    // here we have offsets to shared command array,
    // used by glDrawElementsIndirect,
    // with range (offset, offset + count)
    // and its buffer_id array with range (0, count)
    command_buffer_sub_arena{ Arena(command_buffer_slot) },
    buffer_id_slots { ThreadSafeNumberPool(command_buffer_slot.count) },
    is_enabled{ is_enabled } {
  }
  const unsigned int id;
  // caller must aquire it on write
  mutex mtx;
  GLenum mode;  // GL_TRIANGLES GL_LINES
  Shader* shader;
  unsigned int GetCommandCount() {
    return command_buffer_sub_arena.GetUsed();
  }
  unsigned int GetBaseOffset() {
    return command_buffer_sub_arena.GetBaseOffset();
  }

  unsigned int GetAbsoluteCommandOffset(MeshDataSlots& slots) {
    DEBUG_ASSERT(slots.IsBufferIdAllocated());
    return slots.buffer_id + command_buffer_slot.offset;
  }
private:
  unsigned int GetRelativeBufferId(MemorySlot& sub_command_buffer_slot) {
    return sub_command_buffer_slot.offset - command_buffer_slot.offset;
  }
  void Allocate(MeshDataSlots& out_slots, unsigned int instances_count) {
    MemorySlot sub_command_buffer_slot = command_buffer_sub_arena.Allocate(1);
    out_slots.buffer_id = GetRelativeBufferId(sub_command_buffer_slot);
    DEBUG_ASSERT(out_slots.IsBufferIdAllocated());
  }
  void Release(MeshDataSlots& out_slots) {
    command_buffer_sub_arena.Release({ GetAbsoluteCommandOffset(out_slots), 1 });
  }
};
