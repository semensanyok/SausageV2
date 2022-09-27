#pragma once
#include "sausage.h"
#include "GPUStructs.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
  BufferStorage* buffer = nullptr;
  BufferType::BufferTypeFlag used_buffers;
public:
  BufferConsumer(BufferStorage* buffer, BufferType::BufferTypeFlag used_buffers) :
    buffer{ buffer }
  {
  }
  int GetBufferId() { return buffer->id; }
  void Init();
  void PreDraw();
  void PostDraw();
  int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, int command_offset = -1);
  int AddCommand(DrawElementsIndirectCommand& command, int command_offset = -1);
};
