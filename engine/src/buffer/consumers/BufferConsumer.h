#pragma once
#include "sausage.h"
#include "BufferStorage.h"

class BufferConsumer {
protected:
	BufferStorage* buffer;
	BufferType::BufferTypeFlag used_buffers;
public:
	BufferConsumer(BufferStorage* buffer, BufferType::BufferTypeFlag used_buffers) : 
		buffer{ buffer }, 
		used_buffers{ used_buffers }
	{

	}
	int GetBufferId() { return buffer->id; }
	BufferType::BufferTypeFlag GetUsedBuffers() { return used_buffers; }
	void PreDraw(BufferType::BufferTypeFlag all_batch_used_buffers);
	void PostDraw();
	void ActivateCommandBuffer(CommandBuffer* buf);
	CommandBuffer* CreateCommandBuffer(unsigned int size);
	void RemoveCommandBuffer(CommandBuffer* to_remove);
	int AddCommands(vector<DrawElementsIndirectCommand>& active_commands, CommandBuffer* buf, int command_offset = -1);
	int AddCommand(DrawElementsIndirectCommand& command, CommandBuffer* buf, int command_offset = -1);
};