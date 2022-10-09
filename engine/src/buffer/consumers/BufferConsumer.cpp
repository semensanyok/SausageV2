#include "BufferConsumer.h"

bool BufferConsumer::RequestBuffersOffsets(MeshDataBase* out_mesh, unsigned long vertices_size, unsigned long indices_size) {
    return buffer->RequestBuffersOffsets(out_mesh, vertices_size, indices_size);
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
