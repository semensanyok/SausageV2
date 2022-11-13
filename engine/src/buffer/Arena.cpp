#include "Arena.h"

unsigned int Arena::GetFreeSpace() {
  lock_guard l(mtx);
  return _GetFreeSpace();
}

unsigned int Arena::GetBaseOffset() {
  return base_slot.offset;
}

MemorySlot Arena::Allocate(const unsigned int size) {
  lock_guard l(mtx);
  auto size_to_alloc = _GetSmallestEncompassing(size);
  if (free_gaps_slots.empty()) {
    MemorySlot res = _AllocateNewSlotIfHasSpace(size_to_alloc, size);
    return res;
  }
  else {
    auto maybe_gap = free_gaps_slots.lower_bound({ 0, size_to_alloc, size });
    if (maybe_gap == free_gaps_slots.end()) {
      MemorySlot res = _AllocateNewSlotIfHasSpace(size_to_alloc, size);
      return res;
    }
    bool is_exact = maybe_gap->count == size_to_alloc;
    if (is_exact) {
      MemorySlot res = *maybe_gap;
      free_gaps_slots.erase(maybe_gap);
      return res;
    } else {
      // copy -> erase -> modify -> insert
      auto gap = *maybe_gap;
      free_gaps_slots.erase(maybe_gap);

      MemorySlot res = { gap.offset, size_to_alloc, size };

      gap.count -= size_to_alloc;
      gap.offset += size_to_alloc;
      free_gaps_slots.insert(gap);

      return res;
    }
  }
}

void Arena::Release(MemorySlot slot) {
  lock_guard l(mtx);
  DEBUG_ASSERT(slot.offset + slot.count <= allocated);
  if (slot.offset + slot.count == allocated) {
    allocated -= slot.count;
    return;
  }
  free_gaps_slots.insert({ slot.offset, slot.count, 0 });
}

void Arena::Reset() {
  lock_guard l(mtx);
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

MemorySlot Arena::_AllocateNewSlotIfHasSpace(const unsigned int size_to_alloc, const unsigned int used_size)
{
  // caller responsible for argument correctness
  // size_to_alloc = _GetSmallestEncompassing(size_to_alloc);
  unsigned int free_space = _GetFreeSpace();
  if (size_to_alloc > _GetFreeSpace()) {
    LOG(format("Exceeded free space, size_to_alloc={}, free_space={}", size_to_alloc, free_space));
    return MemorySlots::NULL_SLOT;
  }
  unsigned int offset = allocated + base_slot.offset;
  allocated += size_to_alloc;
  return { offset, size_to_alloc,  used_size };
}

unsigned int Arena::_GetSmallestEncompassing(const unsigned int size) {
  unsigned int res = 0;
  if (slot_size == POWER_OF_TWO)
  {
    res = 1;
  }
  while (res < size) {
    switch (slot_size)
    {
    case ONE:
      return size;
    case POWER_OF_TWO:
      res <<= 1;
      break;
    case FOUR:
      res += 4;
      break;
    default:
      break;
    }
  }
  return res;
}
