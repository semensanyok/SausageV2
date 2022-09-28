#pragma once

#include "sausage.h"
#include "Macros.h"

using namespace std;

struct MemorySlot {
  unsigned long offset;
  unsigned long count;
};

using memory_slot_count_first_comparator =
decltype([](const MemorySlot& lhs, const MemorySlot& rhs) {
  return lhs.count == rhs.count ?
    lhs.offset > rhs.offset : lhs.count > rhs.count;
});

/**
 * @brief allocates slots of powers of 2,
 *  to always be divisible to split large slots for smaller meshes
 * For simplier single number allocation look for ThreadSafeNumberPool.cpp
*/
class Arena {
  const static inline MemorySlot NULL_SLOT = { 0, 0 };
  // Arena::Release must be called only by aquired class
  // to avoid extra validations
  friend class MeshDataBase;

  unsigned int allocated;
  mutex mtx;
  set<MemorySlot, memory_slot_count_first_comparator> free_gaps_slots;
public:
  const unsigned int max;
  // offset into BufferStorage GPU buffer
  const unsigned int offset;

  Arena(
    const unsigned int max,
    const unsigned int offset
  ) : max{ max }, allocated{ 0 }, offset{ offset }  {
  }
  unsigned int GetFreeSpace() {
    lock_guard(mtx);
    return _GetFreeSpace();
  }
  MemorySlot Allocate(const unsigned int size) {
    lock_guard(mtx);
    unsigned int free_space = _GetFreeSpace();
    auto size_encompassing_power_of_2 = _GetSmallestEncompassingPowerOf2(size);
    if (free_gaps_slots.empty()) {
      MemorySlot res = _AllocateNewSlotIfHasSpace(size_encompassing_power_of_2);
      allocated += res.count;
    }
    else {
      auto maybe_gap = free_gaps_slots.lower_bound({ 0, size_encompassing_power_of_2 });
      bool is_exact = maybe_gap->count == size_encompassing_power_of_2;
      if (is_exact) {
        MemorySlot res = *maybe_gap;
        free_gaps_slots.erase(maybe_gap);
        return res;
      }
      else {
        MemorySlot gap = *free_gaps_slots.erase(maybe_gap);
        MemorySlot res = { gap.offset, size_encompassing_power_of_2 };
        gap.count -= size_encompassing_power_of_2;
        gap.offset += size_encompassing_power_of_2;
        free_gaps_slots.insert(gap);
        return res;
      }
    }
  }
  void Reset() {
    lock_guard(mtx);
    allocated = 0;
    free_gaps_slots = {};
  }
private:
  // TODO: call from MeshData friend class
  void Release(MemorySlot slot) {
    lock_guard(mtx);
    DEBUG_ASSERT(slot.offset + slot.count <= allocated);
    if (slot.offset + slot.count == allocated) {
      allocated -= slot.count;
      return;
    }
    free_gaps_slots.insert({ slot.offset, slot.count });
  }

  unsigned int _GetFreeSpace() {
    return max - allocated;
  }
  /**
   * @param size_to_alloc power of 2 slot, encompassing size to allocate
   *        (to reduce amount of calculations)
  */
  MemorySlot _AllocateNewSlotIfHasSpace(const unsigned int size_to_alloc)
  {
    // caller responsible for argument correctness
    // size_to_alloc = _GetSmallestEncompassingPowerOf2(size_to_alloc);
    if (size_to_alloc > _GetFreeSpace()) {
      return NULL_SLOT;
    }
    unsigned int offset = allocated;
    allocated += size_to_alloc;
    return { offset, size_to_alloc };

  }
  unsigned int _GetSmallestEncompassingPowerOf2(const unsigned int size) {
    unsigned int res = 1;
    while (res < size) {
      res <<= 1;
    }
    return res;
  }
};
