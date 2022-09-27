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

// make sure to be multiples of each other, to allocate contigious memory regions in BufferStorage
// numbers interpreted as num of vertices/indices/commands/... in BufferStorage
namespace SausageArena {
  // SMALL
  const unsigned int S_4 = 4;
  const unsigned int S_8 = 8;
  const unsigned int S_16 = 16;
  const unsigned int S_32 = 32;
  const unsigned int S_64 = 64;
  const unsigned int S_128 = 128;
  const unsigned int S_256 = 256;
  const unsigned int S_512 = 512;
  const unsigned int S_1024 = 1024;
  const unsigned int S_2048 = 2048;

  // MEDIUM
  const unsigned int M_4096 = 4096;
  const unsigned int M_8192 = 8192;
  const unsigned int M_16384 = 16384;

  // LARGE
  const unsigned int L_32768 = 32768;
  const unsigned int L_65536 = 65536;

  const unsigned int MAX = L_65536;

  const set<unsigned int> ARENA_SLOTS =
  {
    S_4,
    S_16,
    S_32,
    S_64,
    S_128,
    S_256,
    S_512,
    S_1024,
    S_2048,
    M_4096,
    M_8192,
    M_16384
  };
  // unreasoned, initial guess
  const set<unsigned int> ARENA_SIZES =
  { S_64, S_512, S_1024, M_4096, M_16384, L_65536 };
};


class Arena {
  // Arena::Release must be called only by aquired class
  // to avoid extra validations
  friend class MeshDataBase;

  const unsigned int max;
  unsigned int allocated;
  // offset into BufferStorage GPU buffer
  const unsigned int offset;

  mutex mtx;

  set<MemorySlot, memory_slot_count_first_comparator> allocated_slots;
  set<MemorySlot, memory_slot_count_first_comparator> free_gaps_slots;
public:
  Arena(
    const unsigned int max,
    const unsigned int offset
  ) : max{ max }, allocated{ 0 }, offset{ offset }  {
  }
  unsigned int GetFreeSpace() {
    lock_guard(mtx);
    return max - allocated;
  }
  MemorySlot Allocate(const unsigned int size) {
    lock_guard(mtx);
    DEBUG_ASSERT(GetFreeSpace() >= size);
    MemorySlot res;
    if (free_gaps_slots.empty()) {
      res = _GetNewSlot(size);
      allocated += res.count;
    } else {
      auto maybe_gap = free_gaps_slots.lower_bound({ 0, size });
      if (maybe_gap == free_gaps_slots.end()) {
        res = _GetNewSlot(size);
        allocated += res.count;
      } else {
        res = *maybe_gap;
      }
    }
  }
private:
  void Release(MemorySlot slot) {
    lock_guard(mtx);
    DEBUG_ASSERT(slot.offset + slot.count <= allocated);
    if (slot.offset + slot.count == allocated) {
      allocated -= slot.count;
      return;
    }
    free_gaps_slots.insert({ slot.offset, slot.count });
  }
  MemorySlot _GetNewSlot(const unsigned int size)
  {
    auto lb = SausageArena::ARENA_SLOTS.lower_bound(size);
    if (lb == SausageArena::ARENA_SLOTS.end()) {
      return { 0, 0 };
    }
    else {
      return { offset + allocated, *lb };
    }
  }
};
