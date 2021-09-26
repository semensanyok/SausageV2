#pragma once
#include "sausage.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
	BufferStorage* buffer;
public:
	BufferConsumer(BufferStorage* buffer) : buffer{ buffer } {

	}
	int GetBufferId() { return buffer->id; }
    void PreDraw();
	void PostDraw();
	void ActivateCommandBuffer(CommandBuffer* buf);
	CommandBuffer* CreateCommandBuffer(unsigned int size);
	void RemoveCommandBuffer(CommandBuffer* to_remove);
	int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, CommandBuffer* buf, int command_offset = -1);
	int AddCommand(DrawElementsIndirectCommand& command, CommandBuffer* buf,
		int command_offset = -1);
};