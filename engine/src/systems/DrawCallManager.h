#pragma once

#include "sausage.h"
#include "Macros.h"
#include "ShaderManager.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "DrawCallStruct.h"
#include "Renderer.h"
#include "MeshDataStruct.h"
#include "MeshManager.h"
#include "OverlayStruct.h"

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

  BufferStorage* buffer;

  MeshManager* mesh_manager;

  map<unsigned int, DrawCall*> draw_call_by_id;
  // can add ablitily for each mesh to participate in multiple commands (to use in multiple shaders)
  // for simlicity - keep 1 command for now
  unordered_map<unsigned int, DrawCommandWithMeshMeta> command_by_mesh_id;
  unordered_map<unsigned int, DrawCall*> dc_by_mesh_id;

  DrawCallManager(
    ShaderManager* shader_manager,
    Renderer* renderer,
    BufferStorage* buffer,
    MeshManager* mesh_manager
  ) : renderer{ renderer },
    command_buffer_arena{ new Arena({0, MAX_COMMAND}) },
    buffer{ buffer },
    mesh_manager{ mesh_manager }
  {
    //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
    //    or place shader with dynamic number of meshes at the end
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(GetNumDrawCommandsForFontDrawCall()),
      false
    );
    draw_call_by_id[font_ui_dc->id] = font_ui_dc;

    back_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      command_buffer_arena->Allocate(GetNumDrawCommandsForBackDrawCall()),
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

  MeshDataInstance* AddNewInstance(MeshDataBase* mesh) {
    auto command_iter = command_by_mesh_id.find(mesh->id);
    DEBUG_ASSERT(command_iter != command_by_mesh_id.end());
    auto& command = command_iter->second;
    // TODO: ensure instances_slot reallocation in caller service
    //       or better to do it here, where instance first initiated?
    DEBUG_ASSERT(command.command.instanceCount + 1 <= mesh->slots.instances_slot.count);
    // mesh instance_id = 0 when instanceCount == 1. Thus, postincrement.
    mesh_manager->CreateInstancedMesh(mesh, command.command.instanceCount++);
    mesh->slots.instances_slot.used = command.command.instanceCount;
    buffer->BufferCommand(command.command, command.command_buffer_offset);
  }

  /**
   * @brief sets instance count to base mesh draw command.
   *        this command doesnt modify mesh.instance_id
   *        (to set instance_id automatically use AddNewInstance)
   *        note that mesh.instance_id (gl_InstanceID) is in range [0,instance_count-1]
  */
  void SetInstanceCountToCommand(MeshDataBase* mesh, GLuint instance_count) {
    // TODO: if new instance count doesnt fit in existing transforms slot
    //       - reallocate transform offsets, release transform slot
    //       early exit if current slot is enough
    auto command_iter = command_by_mesh_id.find(mesh->id);
    DEBUG_ASSERT(command_iter != command_by_mesh_id.end());
    auto& command = command_iter->second;
    // TODO: ensure instances_slot reallocation in caller service
    DEBUG_ASSERT(instance_count <= mesh->slots.instances_slot.count);
    command.command.instanceCount = instance_count;
    mesh->slots.instances_slot.used = instance_count;
    buffer->BufferCommand(command.command, command.command_buffer_offset);
  }

  /**
   * make sure to pre allocate expected number of instances
   * for instanced call,
   * to avoid frequent command rebuffer and Arena#aquire/release with each AddNewInstance
   * 
   * @param mesh mesh with command/index/vertex slots and buffer_id
   *        allocated via BufferStorage#AllocateStorage
   * @param dc draw call to assign mesh to
  */
  void AddNewCommandToDrawCall(
    MeshDataBase* mesh,
    DrawCall* dc,
    GLuint instance_count
  ) {
    // validation that command doesnt exist already
    DEBUG_ASSERT(dc_by_mesh_id.find(mesh->id) == dc_by_mesh_id.end());
    DEBUG_ASSERT(command_by_mesh_id.find(mesh->id) == command_by_mesh_id.end());
    DrawCommandWithMeshMeta& command = command_by_mesh_id[mesh->id];
    lock_guard l(dc->mtx);

    MemorySlot slot = dc->Allocate(1, mesh->slots);
    command.command_buffer_offset = slot.offset;

    SetToCommandWithOffsets(command, mesh, instance_count);
  }

  /**
   * @brief set/update mesh offsets to existing mesh draw command
   *        use it:
   *        - for vertex/index/offset/instance_coumt/.. data modification for the command
   *        - if command was created via AddNewCommandToDrawCall
   *          prior to mesh data allocation
   *          and offset setup via BufferStorage#AllocateStorage
   *          (command was created initially with 0 offsets and instance count)
  */
  void SetToCommandWithOffsets(
    MeshDataBase* mesh,
    GLuint instance_count
  ) {
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    SetToCommandWithOffsets(command_by_mesh_id[mesh->id], mesh, instance_count);
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

  void SetToCommandWithOffsets(
    DrawCommandWithMeshMeta& command_with_meta,
    MeshDataBase* mesh,
    GLuint instance_count
  ) {
    auto& command = command_with_meta.command;
    command.instanceCount = instance_count;
    command.count = mesh->slots.index_slot.used;
    command.firstIndex = mesh->slots.index_slot.offset;
    command.baseVertex = mesh->slots.vertex_slot.offset;
    command.baseInstance = mesh->slots.buffer_id;

    buffer->BufferCommand(command, command_with_meta.command_buffer_offset);
  }

  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode, MemorySlot command_buffer_slot, bool is_enabled)
  {
    return new DrawCall(total_draw_calls++, shader, mode, command_buffer_slot, is_enabled);
  }
};
