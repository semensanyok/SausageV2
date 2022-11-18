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

  //DrawCall* overlay_3d_dc;

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
    // TODO: access violation reding location if place mesh_dc first
    //mesh_dc = _CreateDrawCall(
    //  shader_manager->all_shaders->blinn_phong,
    //  GL_TRIANGLES,
    //  buffer->CreateCommandBuffer(MAX_BASE_MESHES),
    //  true
    //);

    //  - each drawcall uses contigious range of commands. Need to allocate in advance for shader.
    //    or place shader with dynamic number of meshes at the end
    font_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->font_ui,
      GL_TRIANGLES,
      buffer->CreateCommandBuffer(GetNumDrawCommandsForFontDrawCall(), ),
      false
    );

    back_ui_dc = _CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      buffer->CreateCommandBuffer(GetNumDrawCommandsForBackDrawCall()),
      false
    );

    //overlay_3d_dc = _CreateDrawCall(
      //shader_manager->all_shaders->overlay_3d,
      //GL_TRIANGLES,
      //buffer->CreateCommandBuffer(1),
      //true
    //);

    physics_debug_dc = _CreateDrawCall(
      shader_manager->all_shaders->bullet_debug,
      GL_LINES,
      buffer->CreateCommandBuffer(1),
      false
    );

    mesh_dc = _CreateDrawCall(
      shader_manager->all_shaders->blinn_phong,
      GL_TRIANGLES,
      buffer->CreateCommandBuffer(MAX_BASE_MESHES),
      true
    );

    //renderer->AddDraw(font_ui_dc, DrawOrder::UI_TEXT);
    //renderer->AddDraw(overlay_3d_dc, DrawOrder::OVERLAY_3D);
    //renderer->AddDraw(back_ui_dc, DrawOrder::UI_BACK);
    //renderer->AddDraw(physics_debug_dc, DrawOrder::PHYS_DEBUG);
    renderer->AddDraw(mesh_dc, DrawOrder::MESH);
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
    lock_guard l(dc->mtx);
    DEBUG_ASSERT(!dc_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(!command_by_mesh_id.contains(mesh->id));
    DrawElementsIndirectCommand& command = command_by_mesh_id[mesh->id];
    dc->Allocate(mesh->slots, 1);
    dc_by_mesh_id[mesh->id] = dc;
    SetToCommandWithOffsets<MESH_TYPE>(mesh, 1, is_alloc_instance_slot);
  }

  template<typename MESH_TYPE>
  MeshDataInstance* AddNewInstance(MeshDataBase* mesh,
    bool is_alloc_instance_slot = true) {
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    auto& command = command_by_mesh_id[mesh->id];
    auto instance_id = command.instanceCount;
    auto instance_count = command.instanceCount + 1;

    if (SetToCommandWithOffsets<MESH_TYPE>(mesh, instance_count, is_alloc_instance_slot)) {
      // mesh instance_id = 0 when instanceCount == 1. Thus, postincrement.
      auto instance = mesh_manager->CreateInstancedMesh(mesh, instance_id);
      return instance;
    }
    return nullptr;
  }

  template<typename MESH_TYPE>
  void DisableCommand(MeshDataBase* mesh) {
    auto draw_call = dc_by_mesh_id.find(mesh->id);
    if (draw_call == dc_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawCall for mesh_id={}", mesh->id));
    } else {
      dc_by_mesh_id.erase(draw_call);
    }
    auto command = command_by_mesh_id.find(mesh->id);
    if (command == command_by_mesh_id.end()) {
      LOG(format("WARN: DisableCommand: Not found DrawArraysIndirectCommand for mesh_id={}", mesh->id));
    }
    else {
      SetToCommandWithOffsets<MESH_TYPE>(mesh, 0);
      command_by_mesh_id.erase(command);
    };
  }

  /**
   * @brief set/update mesh offsets to existing mesh draw command
   *        use it:
   *        - for update command vertex/index/offset/new_instance_count/..
   *        - if command was created via AddNewCommandToDrawCall
   *          prior to mesh data allocation
   *          and offset setup via BufferStorage#AllocateStorage
   *          (command was created initially with 0 offsets and instance count)
   * @brief sets instance count to base mesh draw command.
   *        this command doesnt modify mesh.instance_id
   *        (to set instance_id automatically use AddNewInstance)
   *        note that mesh.instance_id (gl_InstanceID) is in range [0,new_instance_count-1]
  */
  template<typename MESH_TYPE>
  bool SetToCommandWithOffsets(MeshDataBase* mesh,
    GLuint instance_count,
    bool is_alloc_instance_slot = true) {
    DEBUG_ASSERT(command_by_mesh_id.contains(mesh->id));
    DEBUG_ASSERT(dc_by_mesh_id.contains(mesh->id));
    auto& command = command_by_mesh_id[mesh->id];
    auto dc = dc_by_mesh_id[mesh->id];
    bool is_success_slot_alloc = is_alloc_instance_slot ?
      AllocateInstanceSlot<MESH_TYPE>(mesh->slots, instance_count, dc) : true;
    if (is_success_slot_alloc) {
      command.instanceCount = instance_count;
      mesh->slots.instances_slot.used = instance_count;
      _SetToCommandWithOffsets(command, mesh->slots, dc);
      return true;
    }
    return false;
  }
private:

  template<typename MESH_TYPE>
  bool AllocateInstanceSlot(MeshDataSlots& mesh_slots,
    GLuint& new_instance_count,
    DrawCall* dc)
  {
    if (new_instance_count == 0) {
      buffer->ReleaseInstanceSlot<MESH_TYPE>(mesh_slots);
      return true;
    } else if (new_instance_count <= mesh_slots.instances_slot.count) {
    // existing slot already fits requested amount
      return true;
    } else if (mesh_slots.instances_slot != MemorySlots::NULL_SLOT) {
      buffer->ReleaseInstanceSlot<MESH_TYPE>(mesh_slots);
    }
    return buffer->AllocateInstanceSlot<MESH_TYPE>(mesh_slots, new_instance_count);
  }

  void _SetToCommandWithOffsets(
    DrawElementsIndirectCommand& command,
    MeshDataSlots& mesh_slots,
    DrawCall* dc
  ) {
    unsigned int command_offset = dc->GetAbsoluteCommandOffset(mesh_slots);
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
