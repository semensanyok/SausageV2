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
