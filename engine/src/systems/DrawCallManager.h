#pragma once

#include "sausage.h"
#include "Macros.h"
#include "ShaderManager.h"
#include "BufferStorage.h"

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
    return command_count;
  }
  void IncrementCommandCount(int num) {
    DEBUG_ASSERT((command_count + num - command_offset) > (command_count + num_commands_to_reserve_in_buffer));
    command_count += num;
  }
private:
  unsigned int command_count = 0;
  // offset into command_buffer, to draw command_count draw commands
  unsigned int command_offset = 0;
  // for performance, to not rebuffer after each command_count change.
  const int num_commands_to_reserve_in_buffer;
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
    int num_commands_to_reserve_in_buffer = 0) :
    id{ id },
    shader{ shader },
    mode{ mode },
    // number choosen randomly
    num_commands_to_reserve_in_buffer{ num_commands_to_reserve_in_buffer } {
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

  // key == draw call id
  unordered_map<unsigned int, DrawCall*> calls;
  // freed offsets for removed call commands
  unordered_map<unsigned int, set<unsigned int>> calls_slots_offsets;

  DrawCallManager(
    ShaderManager* shader_manager
  ) : shader_manager{ shader_manager } {
    mesh_dc = _CreateDrawCall(
          shader_manager->all_shaders->blinn_phong,
          GL_TRIANGLES
    );
    calls[mesh_dc->id] = mesh_dc;
    back_ui_dc = _CreateDrawCall(
          shader_manager->all_shaders->back_ui,
          GL_TRIANGLES
    );
    calls[back_ui_dc->id] = back_ui_dc;
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES
    );
    calls[font_ui_dc->id] = font_ui_dc;
    physics_debug_dc = _CreateDrawCall(
      shader_manager->all_shaders->bullet_debug,
      GL_LINES
    );
    calls[physics_debug_dc->id] = physics_debug_dc;
    overlay_3d_dc = _CreateDrawCall(
      shader_manager->all_shaders->overlay_3d,
      GL_TRIANGLES
    );
    calls[overlay_3d_dc->id] = overlay_3d_dc;
  }
  void SetCommandToDraw(
    Shader* shader,
    DrawElementsIndirectCommand& command,
    unsigned int in_offset = -1
  ) {
    auto call = calls[shader->id];
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
    auto call = calls[shader->id];
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
  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode)
  {
    return new DrawCall(total_draw_calls++, shader, mode);
  }
};
