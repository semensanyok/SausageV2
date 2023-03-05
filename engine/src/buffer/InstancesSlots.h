#pragma once

#include "sausage.h"
#include "Arena.h"
#include "ThreadSafeNumberPool.h"
#include "MeshDataStruct.h"

using namespace std;

struct InstancesSlots {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  // (or MAX_VERTEX, MAX_INDEX, ...)
  Arena instances_slots;

  inline MemorySlot Allocate(const unsigned int size) {
    return instances_slots.Allocate(size);
  };
  inline void Release(MemorySlot& slot) {
    instances_slots.Release(slot);
    slot = MemorySlots::NULL_SLOT;
  };
  inline void Release(MeshDataSlots& slot) {
    instances_slots.Release(slot.instances_slot);
    slot.instances_slot = MemorySlots::NULL_SLOT;
  };
  inline void Reset() {
    instances_slots.Reset();
  };
};

template<typename T>
struct BufferSlots {
  InstancesSlots instances_slots;
  // opengl generated buffer_id
  // dont confuse with DrawElementsIndirectCommand buffer_id (user provided)
  GLuint buffer_id;
  T* buffer_ptr;
  inline void Reset() {
    instances_slots.Reset();
  };
};


template<typename T>
struct BufferNumberPool {
  // must be allocated with size = MAX_INSTANCES
  // index to uniform data as offset + glInstanceId
  // (or MAX_VERTEX, MAX_INDEX, ...)
  ThreadSafeNumberPool instances_slots;
  GLuint buffer_id;
  T* buffer_ptr;

  inline unsigned int Allocate() {
    return instances_slots.ObtainNumber();
  };
  inline void Release(unsigned int number) {
    instances_slots.ReleaseNumber(number);
  };
  inline void Reset() {
    instances_slots.Reset();
  };
};
