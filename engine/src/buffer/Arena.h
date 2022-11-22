#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Logging.h"

using namespace std;

enum ArenaSlotSize {
  // vertex/index buffers
  POWER_OF_TWO = 0,
  // uniforms offsets
  ONE = 1,
  // command buffer, because must be a multiple of four, or get GL_INVALID_VALUE.
  // @ref void glMultiDrawElementsIndirect(GLenum mode, GLenum type, const void* indirect, GLsizei drawcount, GLsizei stride);
  FOUR = 4
};

struct MemorySlot {
  unsigned long offset;
  unsigned long count;
  // to correctly setup DrawElementsIndirectCommand.count
  // i.e. when allocated 64 slot but actual indices used == 36
  unsigned long used;
};

namespace MemorySlots {
  inline MemorySlot NULL_SLOT = { 0, 0, 0 };
}

// hash function for unordered map
template<> struct std::hash<MemorySlot> {
  size_t operator()(MemorySlot const& t) const {
    return t.offset + t.count;
  }
};

// eq for hashmap/hashset
inline bool operator==(const MemorySlot& lhs, const MemorySlot& rhs) {
  return (lhs.offset == rhs.offset) && (lhs.count == rhs.count);
}
inline bool operator<(const MemorySlot& lhs, const MemorySlot& rhs) {
  return lhs.count == rhs.count ?
    lhs.offset < rhs.offset : lhs.count < rhs.count;
}

/**
 * @brief allocates slots
 *
 * optionally of powers of 2:
 * - to always be divisible, to split large slots for smaller meshes.
 * - expected to be advantageous for vertex/index slots management,
 *   which are frequently allocated/deallocated,
 *   and mostly of different sizes
 *   and single vertex/index buffer is used in all kinds of shaders/commands
 * - should not be used for command slots (Arena* instances_slots; in BufferStorage.cpp),
 *   as most of them have constant count
 *   i.e. ui buffer - constant number of commands (draw elements) are compile time const
 *   back/debug/overlay - 1 draw command
 *   only dynamic command buffer is mesh draw call
 * 
 * For simplier single number allocation look for ThreadSafeNumberPool.cpp
*/
class Arena {
  // Arena::Release must be called only by aquired class
  // to avoid extra validations
  friend class MeshDataBase;

  unsigned int allocated;
  mutex mtx;
  set<MemorySlot> free_gaps_slots;
  MemorySlot base_slot;
  ArenaSlotSize slot_size;
public:

  Arena(MemorySlot slot, ArenaSlotSize slot_size = ArenaSlotSize::ONE) :
    base_slot{ slot }, allocated{ 0 }, slot_size{ slot_size }{
  }
  inline unsigned int GetUsed() { return allocated; };
  unsigned int GetFreeSpace();
  unsigned int GetBaseOffset();
  unsigned int GetSize() { return base_slot.count; };
  MemorySlot Allocate(const unsigned int size);
  void Release(MemorySlot slot);
  void Reset();
private:
  unsigned int _GetFreeSpace();
  /**
   * @param size_to_alloc power of 2 slot, encompassing size to allocate
   *        (to reduce amount of calculations)
  */
  MemorySlot _AllocateNewSlotIfHasSpace(const unsigned int size_to_alloc, const unsigned int used_size);
  unsigned int _GetSmallestEncompassing(const unsigned int size);
};
