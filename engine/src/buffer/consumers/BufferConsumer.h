#pragma once
#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
  BufferStorage* buffer = nullptr;
  BufferType::BufferTypeFlag used_buffers;
public:
  BufferConsumer(BufferStorage* buffer,
    BufferType::BufferTypeFlag used_buffers) :
      buffer{ buffer }
  {
  }
  bool RequestBuffersOffsets(
    MeshDataBase* out_mesh,
    unsigned long vertices_size,
    unsigned long indices_size
  );
  void Init();
  void PreDraw();
  void PostDraw();
};
