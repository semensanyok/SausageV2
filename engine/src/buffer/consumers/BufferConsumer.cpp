#include "BufferConsumer.h"

void BufferConsumer::PreDraw() {
    buffer->SyncGPUBufAndUnmap();
    buffer->BindMeshVAOandBuffers(); // TODO: one buffer, no rebind
}

void BufferConsumer::PostDraw() {
    buffer->fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
    buffer->MapBuffers();
}

void BufferConsumer::ActivateCommandBuffer(CommandBuffer* buf) {
    buffer->ActivateCommandBuffer(buf);
}

CommandBuffer* BufferConsumer::CreateCommandBuffer(unsigned int size) {
    return buffer->CreateCommandBuffer(size);
}

void BufferConsumer::RemoveCommandBuffer(CommandBuffer* to_remove) {
    buffer->RemoveCommandBuffer(to_remove);
}

int BufferConsumer::AddCommands(vector<DrawElementsIndirectCommand>& active_commands, CommandBuffer* buf, int command_offset) {
    return buffer->AddCommands(active_commands, buf, command_offset);
}

int BufferConsumer::AddCommand(DrawElementsIndirectCommand& command, CommandBuffer* buf, int command_offset) {
    return buffer->AddCommand(command, buf, command_offset);
}
