#pragma once

#include "sausage.h"
#include "Macros.h"
#include "ShaderManager.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "DrawCallStruct.h"
#include "Renderer.h"

using namespace std;

struct DrawCommandWithMeshMeta {
  DrawElementsIndirectCommand command;
  unsigned int command_buffer_offset;
};

class DrawCallManager {
  Renderer* renderer;
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
  unordered_map<unsigned int, DrawCommandWithMeshMeta> command_by_mesh_id;
  unordered_map<unsigned int, DrawCall*> dc_by_mesh_id;

  DrawCallManager(
    ShaderManager* shader_manager,
    Renderer* renderer
  ) : renderer{ renderer } {
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1),
      false
    );
    draw_call_by_id[font_ui_dc->id] = font_ui_dc;

    back_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1),
      false
    );
    draw_call_by_id[back_ui_dc->id] = back_ui_dc;

    overlay_3d_dc = _CreateDrawCall(
      shader_manager->all_shaders->overlay_3d,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(1),
      true
    );
    draw_call_by_id[overlay_3d_dc->id] = overlay_3d_dc;

    physics_debug_dc = _CreateDrawCall(
      shader_manager->all_shaders->bullet_debug,
      GL_LINES,
      command_buffer_arena->Allocate(1),
      false
    );
    draw_call_by_id[physics_debug_dc->id] = physics_debug_dc;

    mesh_dc = _CreateDrawCall(
      shader_manager->all_shaders->blinn_phong,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(MAX_BASE_MESHES),
      true
    );
    draw_call_by_id[mesh_dc->id] = mesh_dc;

    renderer->AddDraw(font_ui_dc, DrawOrder::UI_TEXT);
    renderer->AddDraw(overlay_3d_dc, DrawOrder::OVERLAY_3D);
    renderer->AddDraw(back_ui_dc, DrawOrder::UI_BACK);
    renderer->AddDraw(physics_debug_dc, DrawOrder::PHYS_DEBUG);
    renderer->AddDraw(mesh_dc, DrawOrder::MESH);
  }

  void AddNewInstanceSetInstanceId(MeshDataBase* mesh) {
    auto command_iter = command_by_mesh_id.find(mesh->id);
    DEBUG_ASSERT(command_iter != command_by_mesh_id.end());
    auto& command = command_iter->second;
    // mesh instance_id = 0 when instanceCount == 1. Thus, postincrement.
    mesh->instance_id = command.command.instanceCount++;
    BufferStorage::GetInstance()->BufferCommand(command.command, command.command_buffer_offset);
  }

  /**
   * @brief sets instance count to base mesh draw command.
   *        this command doesnt modify mesh.instance_id
   *        (to set instance_id automatically use AddNewInstanceSetInstanceId)
   *        note that mesh.instance_id (gl_InstanceID) is in range [0,instance_count-1]
  */
  void SetInstanceCountToCommand(MeshDataBase* mesh, GLuint instance_count) {
    auto command_iter = command_by_mesh_id.find(mesh->id);
    DEBUG_ASSERT(command_iter != command_by_mesh_id.end());
    auto& command = command_iter->second;
    command.command.instanceCount = instance_count;
    BufferStorage::GetInstance()->BufferCommand(command.command, command.command_buffer_offset);
  }


  /**
   * @param out_mesh mesh with command/index/vertex slots and buffer_id
   *        allocated via BufferStorage#RequestBuffersOffsets
   * @param dc draw call to assign mesh to
  */
  void AddNewCommandToDrawCall(
    MeshDataBase* out_mesh,
    DrawCall* dc
  ) {
    // validation that command doesnt exist already
    DEBUG_EXPR(
      auto draw_call = dc_by_mesh_id.find(out_mesh->id);
      DEBUG_ASSERT(draw_call == dc_by_mesh_id.end());
      auto draw_command = command_by_mesh_id.find(out_mesh->id);
      DEBUG_ASSERT(draw_command == command_by_mesh_id.end());
    );
    DrawCommandWithMeshMeta& command = command_by_mesh_id[out_mesh->id];
    lock_guard(dc->mtx);

    MemorySlot slot = dc->Allocate(1);
    command.command_buffer_offset = slot.offset;

    SetToCommandWithOffsets(command, out_mesh, 1);
  }

  void SetToCommandWithOffsets(
    MeshDataBase* mesh,
    GLuint instance_count
  ) {
    auto draw_command = command_by_mesh_id.find(mesh->id);
    DEBUG_ASSERT(draw_command != command_by_mesh_id.end());
    SetToCommandWithOffsets(draw_command->second, mesh, instance_count);
  }

  void SetToCommandWithOffsets(
    DrawCommandWithMeshMeta& command_with_meta,
    MeshDataBase* mesh,
    GLuint instance_count
  ) {
    auto& command = command_with_meta.command;
    command.instanceCount = instance_count;
    command.count = mesh->index_slot.count;
    command.firstIndex = mesh->index_slot.offset;
    command.baseVertex = mesh->vertex_slot.offset;
    command.baseInstance = mesh->buffer_id;

    BufferStorage::GetInstance()->BufferCommand(command, command_with_meta.command_buffer_offset);
  }

  void DisableCommand(MeshDataBase* mesh) {
    auto draw_call = dc_by_mesh_id.find(mesh->id);
    if (draw_call == dc_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawCall for mesh_id={}", mesh->id));
      return;
    } else {
      dc_by_mesh_id.erase(draw_call);
    }
    SetInstanceCountToCommand(mesh, 0);
    auto draw_command = command_by_mesh_id.find(mesh->id);
    if (draw_command == command_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawArraysIndirectCommand for mesh_id={}", mesh->id));
    }
    else {
      command_by_mesh_id.erase(draw_command);
    }
    draw_call->second->Release({ draw_command->second.command_buffer_offset, 1 });
  }
private:
  int total_draw_calls = 0;
  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode, MemorySlot command_buffer_slot, bool is_enabled)
  {
    return new DrawCall(total_draw_calls++, shader, mode, command_buffer_slot, is_enabled);
  }
};
