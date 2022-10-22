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

class DrawCall {
  friend class DrawCallManager;
  MemorySlot command_buffer_slot;
  Arena* command_buffer_sub_arena;
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
    command_buffer_sub_arena{ new Arena(command_buffer_slot) },
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
    DEBUG_ASSERT(slot != MemorySlots::NULL_SLOT);
    out_slots.buffer_id
    return slot;
  }
  void Release(MemorySlot slot) {
    command_buffer_sub_arena->Release(slot);
  }
};
