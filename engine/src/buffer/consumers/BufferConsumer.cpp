#include "BufferConsumer.h"

void BufferConsumer::Init()
{
  buffer->AddUsedBuffers(used_buffers);
}

void BufferConsumer::PreDraw()
{
}

void BufferConsumer::PostDraw()
{
}

int BufferConsumer::AddCommand(DrawElementsIndirectCommand& command,
    DrawCall* out_call,
    MeshDataBase* out_mesh) {
  if (buffer->RequestStorageSetOffsets()) {
    draw_call_manager->AllocateAndBufferCommand(command, call, mesh);
  }
  else {
    DEBUG_EXPR(LOG("Unable to allocate mesh draw command"));
  }
}
