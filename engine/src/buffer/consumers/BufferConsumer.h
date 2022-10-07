#pragma once
#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
  BufferStorage* buffer = nullptr;
  BufferType::BufferTypeFlag used_buffers;
public:
  DrawCallManager* draw_call_manager;
  BufferConsumer(BufferStorage* buffer,
    BufferType::BufferTypeFlag used_buffers) :
      buffer{ buffer }
  {
  }
  void Init();
  void PreDraw();
  void PostDraw();
};
