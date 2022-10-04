#pragma once
#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"
#include "DrawCallManager.h"

class BufferConsumer {
protected:
  BufferStorage* buffer = nullptr;
  BufferType::BufferTypeFlag used_buffers;
public:
  DrawCallManager* draw_call_manager;
  BufferConsumer(BufferStorage* buffer,
    DrawCallManager* draw_call_manager,
    BufferType::BufferTypeFlag used_buffers) :
      buffer{ buffer },
      draw_call_manager{ draw_call_manager }
  {
  }
  void Init();
  void PreDraw();
  void PostDraw();
};
