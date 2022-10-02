#pragma once

#include "sausage.h"
#include "Macros.h"
#include "ShaderManager.h"
#include "BufferStorage.h"
#include "Logging.h"

/**
 * DrawCall (Shader) uses contigious part of commands from indirect command buffer
*/

class Shader;

using namespace std;

class DrawCall {
  MemorySlot command_buffer_slot;
  Arena* command_buffer_sub_arena;
public:
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
  unsigned int id;
  // caller must aquire it on write
  mutex mtx;
  GLenum mode;  // GL_TRIANGLES GL_LINES
  Shader* shader;
  unsigned int GetCommandCount() {
    return command_buffer_sub_arena->GetUsed();
  }
  unsigned int GetBaseOffset() {
    return command_buffer_sub_arena->GetBaseOffset();
  }
  MemorySlot Allocate(unsigned int command_count) {
    MemorySlot slot = command_buffer_sub_arena->Allocate(command_count);
    DEBUG_ASSERT(slot != Arena::NULL_SLOT);
    return slot;
  }
  void Release(MemorySlot slot) {
    command_buffer_sub_arena->Release(slot);
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
  unordered_map<unsigned int, DrawElementsIndirectCommand> command_by_mesh_id;
  unordered_map<unsigned int, DrawCall*> dc_by_mesh_id;

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
  void AllocateAndBufferCommand(
    DrawElementsIndirectCommand& command,
    DrawCall* out_call,
    MeshDataBase* out_mesh
  ) {
    lock_guard(out_call->mtx);
    if (out_mesh->command_offset == -1) {
      MemorySlot slot = out_call->Allocate(1);
      out_mesh->command_offset = slot.offset;
    }
    buffer->BufferCommand(command, out_mesh->command_offset);
  }
  void DisableCommand(MeshDataBase* mesh) {
    // skip validation
    
    //auto draw_command = command_by_mesh_id.find(mesh->id);
    //if (draw_command == command_by_mesh_id.end()) {
    //  LOG(format("Skip DisableCommand. not found DrawArraysIndirectCommand for mesh_id={}", mesh->id));
    //  return;
    //}
    auto draw_call = dc_by_mesh_id.find(mesh->id);
    if (draw_call == dc_by_mesh_id.end()) {
      LOG(format("Skip DisableCommand. not found DrawCall for mesh_id={}", mesh->id));
      return;
    }
    auto empty_command = DrawElementsIndirectCommand();
    AllocateAndBufferCommand(
      empty_command,
      draw_call->second,
      mesh);
    draw_call->second->Release({ (unsigned long)mesh->command_offset, 1 });
  }
private:
  int total_draw_calls = 0;
  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode, MemorySlot command_buffer_slot)
  {
    return new DrawCall(total_draw_calls++, shader, mode, command_buffer_slot);
  }
};
