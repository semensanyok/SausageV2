#include "Arena.h"

unsigned int Arena::GetFreeSpace() {
  lock_guard(mtx);
  return _GetFreeSpace();
}

MemorySlot Arena::Allocate(const unsigned int size) {
  lock_guard(mtx);
  unsigned int free_space = _GetFreeSpace();
  auto size_encompassing_power_of_2 = _GetSmallestEncompassingPowerOf2(size);
  if (free_gaps_slots.empty()) {
    MemorySlot res = _AllocateNewSlotIfHasSpace(size_encompassing_power_of_2);
    return res;
  }
  else {
    auto maybe_gap = free_gaps_slots.lower_bound({ 0, size_encompassing_power_of_2 });
    if (maybe_gap == free_gaps_slots.end()) {
      MemorySlot res = _AllocateNewSlotIfHasSpace(size_encompassing_power_of_2);
      return res;
    }
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

void Arena::Release(MemorySlot slot) {
  lock_guard(mtx);
  DEBUG_ASSERT(slot.offset + slot.count <= allocated);
  if (slot.offset + slot.count == allocated) {
    allocated -= slot.count;
    return;
  }
  free_gaps_slots.insert({ slot.offset, slot.count });
}

void Arena::Reset() {
  lock_guard(mtx);
  allocated = 0;
  free_gaps_slots = {};
}

unsigned int Arena::_GetFreeSpace() {
  return base_slot.count - allocated;
}

/**
* @param size_to_alloc power of 2 slot, encompassing size to allocate
*        (to reduce amount of calculations)
*/

MemorySlot Arena::_AllocateNewSlotIfHasSpace(const unsigned int size_to_alloc)
{
  // caller responsible for argument correctness
  // size_to_alloc = _GetSmallestEncompassingPowerOf2(size_to_alloc);
  if (size_to_alloc > _GetFreeSpace()) {
    return NULL_SLOT;
  }
  unsigned int offset = allocated + base_slot.offset;
  allocated += size_to_alloc;
  return { offset, size_to_alloc };
}

unsigned int Arena::_GetSmallestEncompassingPowerOf2(const unsigned int size) {
  unsigned int res = 1;
  while (res < size) {
    res <<= 1;
  }
  return res;
}
