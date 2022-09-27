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

int BufferConsumer::AddCommands(vector<DrawElementsIndirectCommand>& active_commands, int command_offset) {
    return buffer-> BufferCommands(active_commands, command_offset);
}

int BufferConsumer::AddCommand(DrawElementsIndirectCommand& command, int command_offset) {
    return buffer->BufferCommand(command, command_offset);
}
