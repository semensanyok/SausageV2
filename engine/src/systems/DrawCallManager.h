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

class DrawCallManager {
  Renderer* renderer;
public:
  DrawCall* mesh_dc;

  DrawCall* back_ui_dc;
  DrawCall* font_ui_dc;

  DrawCall* overlay_3d_dc;

  DrawCall* physics_debug_dc;

  BufferStorage* buffer;

  MeshManager* mesh_manager;

  // can add ablitily for each mesh to participate in multiple commands (to use in multiple shaders)
  // for simlicity - keep 1 command for now
  unordered_map<unsigned int, DrawElementsIndirectCommand> command_by_mesh_id;
  unordered_map<unsigned int, DrawCall*> dc_by_mesh_id;

  int total_draw_calls = 0;

  DrawCallManager(
    ShaderManager* shader_manager,
    Renderer* renderer,
    BufferStorage* buffer,
    MeshManager* mesh_manager
  ) : renderer{ renderer },
    buffer{ buffer },
    mesh_manager{ mesh_manager }
  {
    //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
    //    or place shader with dynamic number of meshes at the end
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES,
      buffer->AllocateCommandBufferSlot(GetNumDrawCommandsForFontDrawCall()),
      false
    );

    back_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      buffer->AllocateCommandBufferSlot(GetNumDrawCommandsForBackDrawCall()),
      false
    );

    overlay_3d_dc = _CreateDrawCall(
      shader_manager->all_shaders->overlay_3d,
      GL_TRIANGLES,
      buffer->AllocateCommandBufferSlot(1),
      true
    );

    physics_debug_dc = _CreateDrawCall(
      shader_manager->all_shaders->bullet_debug,
      GL_LINES,
      buffer->AllocateCommandBufferSlot(1),
      false
    );

    mesh_dc = _CreateDrawCall(
      shader_manager->all_shaders->blinn_phong,
      GL_TRIANGLES,
      buffer->AllocateCommandBufferSlot(MAX_BASE_MESHES),
      true
    );

    renderer->AddDraw(font_ui_dc, DrawOrder::UI_TEXT);
    renderer->AddDraw(overlay_3d_dc, DrawOrder::OVERLAY_3D);
    renderer->AddDraw(back_ui_dc, DrawOrder::UI_BACK);
    renderer->AddDraw(physics_debug_dc, DrawOrder::PHYS_DEBUG);
    renderer->AddDraw(mesh_dc, DrawOrder::MESH);
  }

  template<typename MESH_TYPE>
  MeshDataInstance* AddNewInstance(MeshDataBase* mesh) {
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(dc_by_mesh_id.contains(mesh->id));
    auto& command = command_by_mesh_id[mesh->id];
    auto dc = dc_by_mesh_id[mesh->id];
    // reallocate if
    bool is_success_slot_alloc = true;
    auto instance_count = command.instanceCount + 1;
    if (instance_count > mesh->slots.instances_slot.count) {
      is_success_slot_alloc = AllocateInstanceSlot<MESH_TYPE>(mesh->slots,
        instance_count, command, dc);
    }
    if (is_success_slot_alloc) {
      // mesh instance_id = 0 when instanceCount == 1. Thus, postincrement.
      auto instance = mesh_manager->CreateInstancedMesh(mesh, command.instanceCount++);
      mesh->slots.instances_slot.used = command.instanceCount;
      buffer->BufferCommand(command, dc->GetAbsoluteCommandOffset(mesh->slots));
      return instance;
    }
    return nullptr;
  }

  /**
   * @brief sets instance count to base mesh draw command.
   *        this command doesnt modify mesh.instance_id
   *        (to set instance_id automatically use AddNewInstance)
   *        note that mesh.instance_id (gl_InstanceID) is in range [0,new_instance_count-1]
  */
  template<typename MESH_TYPE>
  void SetInstanceCountToCommand(MeshDataBase* mesh, GLuint instance_count) {
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(dc_by_mesh_id.contains(mesh->id));
    auto& command = command_by_mesh_id[mesh->id];
    auto dc = dc_by_mesh_id[mesh->id];
    bool is_success_slot_alloc = true;
    if (instance_count > mesh->slots.instances_slot.count) {
      is_success_slot_alloc = AllocateInstanceSlot<MESH_TYPE>(mesh->slots,
        instance_count, command, dc);
    }
    if (instance_count == 0) {
      buffer->ReleaseInstanceSlot<MESH_TYPE>(mesh->slots);
    }
    if (is_success_slot_alloc) {
      command.instanceCount = instance_count;
      mesh->slots.instances_slot.used = instance_count;
      buffer->BufferCommand(command, dc->GetAbsoluteCommandOffset(mesh->slots));
    }
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
  template<typename MESH_TYPE>
  void AddNewCommandToDrawCall(
    MeshDataBase* mesh,
    DrawCall* dc,
    GLuint instance_count,
    // bullet debug drawer doesnt use instance slot in any ssbo, only vertex-index
    bool is_alloc_instance_slot = true
  ) {
    DEBUG_ASSERT(!dc_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(!command_by_mesh_id.contains(mesh->id));
    DrawElementsIndirectCommand& command = command_by_mesh_id[mesh->id];
    lock_guard l(dc->mtx);

    dc->Allocate(mesh->slots, 1);
    if (buffer->AllocateInstanceSlot<MESH_TYPE>(mesh->slots, instance_count)) {
      _SetToCommandWithOffsets(command, mesh->slots, dc->GetAbsoluteCommandOffset(mesh->slots));
    }
  }

  /**
   * @brief set/update mesh offsets to existing mesh draw command
   *        use it:
   *        - for update command vertex/index/offset/new_instance_count/..
   *        - if command was created via AddNewCommandToDrawCall
   *          prior to mesh data allocation
   *          and offset setup via BufferStorage#AllocateStorage
   *          (command was created initially with 0 offsets and instance count)
  */
  template<typename MESH_TYPE>
  void _SetToCommandWithOffsets(
    MeshDataBase* mesh,
    GLuint instance_count,
    // bullet debug drawer doesnt use instance slot in any ssbo, only vertex-index
    bool is_alloc_instance_slot = true
  ) {
    DEBUG_ASSERT(dc_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    auto dc = dc_by_mesh_id[mesh->id];
    auto command = command_by_mesh_id[mesh->id];
    // if asked more then current slots have - reallocate
    bool is_success_slot_alloc = false;
    if (is_alloc_instance_slot && instance_count > mesh->slots.instances_slot.count) {
      is_success_slot_alloc = AllocateInstanceSlot<MESH_TYPE>(mesh->slots, instance_count, command, dc);
    }
    else {
      mesh->slots.instances_slot.used = instance_count;
      is_success_slot_alloc = true;
    }
    if (is_success_slot_alloc) {
      _SetToCommandWithOffsets(command_by_mesh_id[mesh->id], mesh->slots,
        dc->GetAbsoluteCommandOffset(mesh->slots));
    }
  }

  template<typename MESH_TYPE>
  void DisableCommand(MeshDataBase* mesh) {
    auto draw_call = dc_by_mesh_id.find(mesh->id);
    if (draw_call == dc_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawCall for mesh_id={}", mesh->id));
      return;
    } else {
      dc_by_mesh_id.erase(draw_call);
    }
    auto draw_command = command_by_mesh_id.find(mesh->id);
    if (draw_command == command_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawArraysIndirectCommand for mesh_id={}", mesh->id));
    }
    else {
      SetInstanceCountToCommand<MESH_TYPE>(mesh, 0);
      command_by_mesh_id.erase(draw_command);
    };
    draw_call->second->Release(mesh->slots);
  }
private:

  template<typename MESH_TYPE>
  bool AllocateInstanceSlot(MeshDataSlots& mesh_slots,
    GLuint& new_instance_count,
    DrawElementsIndirectCommand& command,
    DrawCall* dc)
  {
    if (new_instance_count <= mesh_slots.instances_slot.count) {
      return false;
    }
    if (mesh_slots.instances_slot != MemorySlots::NULL_SLOT) {
      buffer->ReleaseInstanceSlot<MESH_TYPE>(mesh_slots);
    }
    return buffer->AllocateInstanceSlot<MESH_TYPE>(mesh_slots, new_instance_count);
  }

  void _SetToCommandWithOffsets(
    DrawElementsIndirectCommand& command,
    MeshDataSlots& mesh_slots,
    unsigned int command_offset
  ) {
    command.instanceCount = mesh_slots.instances_slot.count;
    command.count = mesh_slots.index_slot.used;
    command.firstIndex = mesh_slots.index_slot.offset;
    command.baseVertex = mesh_slots.vertex_slot.offset;
    command.baseInstance = mesh_slots.buffer_id;

    buffer->BufferCommand(command, command_offset);
  }

  DrawCall* _CreateDrawCall(Shader* shader, GLenum mode,
    MemorySlot command_buffer_slot, bool is_enabled)
  {
    return new DrawCall(total_draw_calls++, shader, mode, command_buffer_slot, is_enabled);
  }
};
