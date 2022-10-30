#include "BufferConsumer.h"

bool BufferConsumer::AllocateStorage(MeshDataSlots& out_slots,
  unsigned long vertices_size,
  unsigned long indices_size,
  unsigned long num_instances) {
  return buffer->AllocateStorage(out_slots, vertices_size, indices_size);
}

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
