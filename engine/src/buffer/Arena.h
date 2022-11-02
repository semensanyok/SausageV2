#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Logging.h"

using namespace std;

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
    lhs.offset > rhs.offset : lhs.count > rhs.count;
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
  const bool is_allocate_powers_of_2;
public:

  Arena(MemorySlot slot, bool is_allocate_powers_of_2 = false) :
    base_slot{ slot }, allocated{ 0 }, is_allocate_powers_of_2{ is_allocate_powers_of_2 }{
  }
  inline unsigned int GetUsed() { return allocated; };
  unsigned int GetFreeSpace();
  unsigned int GetBaseOffset();
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
  unsigned int _GetSmallestEncompassingPowerOf2(const unsigned int size);
};
