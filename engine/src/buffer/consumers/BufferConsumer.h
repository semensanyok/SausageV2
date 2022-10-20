#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
  BufferStorage* buffer;
  BufferType::BufferTypeFlag used_buffers;
public:
  BufferConsumer(BufferStorage* buffer,
    BufferType::BufferTypeFlag used_buffers) :
      buffer{ buffer }
  {
  }
  bool AllocateStorage(
    MeshDataSlots& out_slots,
    unsigned long vertices_size,
    unsigned long indices_size,
    unsigned long num_instances
  );
  void Init();
  void PreDraw();
  void PostDraw();
};
