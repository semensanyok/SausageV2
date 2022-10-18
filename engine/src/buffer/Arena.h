#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Logging.h"

using namespace std;

namespace MemorySlots {
  inline MemorySlot NULL_SLOT = { 0, 0, 0 };
}

struct MemorySlot {
  unsigned long offset;
  unsigned long count;
  // to correctly setup DrawElementsIndirectCommand.count
  // i.e. when allocated 64 slot but actual indices used == 36
  unsigned long used;
};

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
 * @brief allocates slots of powers of 2,
 *  to always be divisible to split large slots for smaller meshes
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
public:

  Arena(MemorySlot slot) : base_slot{ slot }, allocated{ 0 }  {
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
