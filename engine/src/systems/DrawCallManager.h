#pragma once

#include "sausage.h"
#include "Macros.h"
#include "ShaderManager.h"
#include "BufferStorage.h"

/**
 * DrawCall (Shader) uses contigious part of commands from indirect command buffer
*/

class Shader;

using namespace std;

class DrawCall {
  friend class DrawCallManager;
  friend class Renderer;
public:
  const unsigned int id;
  // caller must aquire it on write
  mutex mtx;
  GLenum mode = GL_TRIANGLES;  // GL_TRIANGLES GL_LINES
  Shader* shader = nullptr;
  unsigned int GetCommandCount() {
    return commands_used;
  }
  void IncrementCommandCount(int num) {
    DEBUG_ASSERT((commands_used + num) < (command_buffer_slot.count));
    commands_used += num;
  }
private:
  MemorySlot command_buffer_slot;
  Arena* command_buffer_sub_arena;
  unsigned int commands_used = 0;
  /**
   * @brief
   * @param shader
   * @param mode
   * @param is_reserve_command_count_in_buffer
   *            set this flag when expect
   *          to add more DrawElementsIndirectCommand
   *          to this draw_call.
   *            false when not expect to change.
   *          i.e. single instanced draw call, where only instance count changes.
  */
  DrawCall(unsigned int id,
    Shader* shader,
    GLenum mode,
    MemorySlot command_buffer_slot) :
    id{ id },
    shader{ shader },
    mode{ mode },
    command_buffer_slot{ command_buffer_slot },
    // offset is 0 because 
    command_buffer_sub_arena{ new Arena(command_buffer_slot) } {
  }
};


class DrawCallManager {
  ShaderManager* shader_manager;
  BufferStorage* buffer;
public:
  DrawCall* mesh_dc;

  DrawCall* back_ui_dc;
  DrawCall* font_ui_dc;

  DrawCall* overlay_3d_dc;

  DrawCall* physics_debug_dc;

  Arena* command_buffer_arena;

  map<unsigned int, DrawCall*> draw_call_by_id;
  // can add ablitily for each mesh to participate in multiple commands (to use in multiple shaders)
  // for simlicity - keep 1 command for now
  unordered_map<unsigned int, DrawArraysIndirectCommand> command_by_mesh_id;

  DrawCallManager(
    ShaderManager* shader_manager
  ) : shader_manager{ shader_manager } {
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1)
    );
    draw_call_by_id[font_ui_dc->id] = font_ui_dc;

    overlay_3d_dc = _CreateDrawCall(
      shader_manager->all_shaders->overlay_3d,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1)
    );
    draw_call_by_id[overlay_3d_dc->id] = overlay_3d_dc;

    back_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1)
    );
    draw_call_by_id[back_ui_dc->id] = back_ui_dc;

    physics_debug_dc = _CreateDrawCall(
      shader_manager->all_shaders->bullet_debug,
      GL_LINES,
      command_buffer_arena->Allocate(1)
    );
    draw_call_by_id[physics_debug_dc->id] = physics_debug_dc;

    // dynamic commands number for mesh draw call
    // thus, set it to the end
    mesh_dc = _CreateDrawCall(
      shader_manager->all_shaders->blinn_phong,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(MAX_BASE_MESHES)
    );
    draw_call_by_id[mesh_dc->id] = mesh_dc;
  }
  void SetCommandToDraw(
    Shader* shader,
    DrawElementsIndirectCommand& command,
    unsigned int in_offset = -1
  ) {
    auto call = draw_call_by_id[shader->id];
    lock_guard(call->mtx);
    unsigned int offset = in_offset == -1 ? call->GetCommandCount() : in_offset;
    buffer->BufferCommand(command, offset);
    if (in_offset == -1) {
      call->IncrementCommandCount(1);
    }
  }
  void SetCommandsToDraw(
    Shader* shader,
    vector<DrawElementsIndirectCommand> commands,
    unsigned int in_offset = -1
  ) {
    auto call = draw_call_by_id[shader->id];
    lock_guard(call->mtx);
    unsigned int offset = offset == -1 ? call->GetCommandCount() : offset;
    buffer->BufferCommands(commands, offset);
    if (in_offset == -1) {
      call->IncrementCommandCount(commands.size());
    }
  }
  void DisableCommand(Shader* shader, DrawElementsIndirectCommand command, unsigned int in_offset) {
    command.instanceCount = 0;
    calls_slots_offsets.insert({ in_offset, {} });
    SetCommandToDraw(shader, command, in_offset);
    auto a = calls_slots_offsets[in_offset];
    a.insert(in_offset);
  }
private:
  int total_draw_calls = 0;
  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode, MemorySlot command_buffer_slot)
  {
    return new DrawCall(total_draw_calls++, shader, mode, command_buffer_slot);
  }
};
