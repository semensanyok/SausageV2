#include "BufferConsumer.h"

bool BufferConsumer::AllocateStorage(MeshDataBase* out_mesh, unsigned long vertices_size, unsigned long indices_size) {
    return buffer->AllocateStorage(out_mesh, vertices_size, indices_size);
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
