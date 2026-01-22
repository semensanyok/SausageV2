#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "Macros.h"
#include "Vertex.h"
#include "MeshDataStruct.h"
#include "GLCommandBuffers.h"

using namespace std;
class Shader;

/**
 * make sure to group VertexType in contigious series of draws
 */
enum class DrawOrder {
  //VertexType::STATIC
  TERRAIN,
  MESH_STATIC,
  OVERLAY_3D,
  //VertexType::MESH
  MESH,
  //VERTEX_TYPE::UI
  UI_BACK,
  UI_TEXT,
  //VertexType::OUTLINE
  OUTLINE
};

inline VertexType GetVertexTypeByDrawOrder(DrawOrder draw_order) {
  switch (draw_order)
  {
  case DrawOrder::MESH:
    return VertexType::MESH;
    break;
  case DrawOrder::OVERLAY_3D:
  case DrawOrder::MESH_STATIC:
  case DrawOrder::TERRAIN:
    return VertexType::STATIC;
    break;
  case DrawOrder::UI_BACK:
  case DrawOrder::UI_TEXT:
    return VertexType::UI;
    break;
  case DrawOrder::OUTLINE:
    return VertexType::OUTLINE;
    break;
  default:
    break;
  }
};

// MeshDataClass is used to determine which buffer to use to allocate instance_id for shader
// (referenced from uniform_offset[buffer_id] -> instance_id to fetch textures/transforms)
// commented out because now it is used only in template function
//template<typename MeshDataClass>
class DrawCall {
  friend class DrawCallManager;
  friend class SpatialManager;
  friend class Renderer;
public:
  bool is_dirty = false;
  bool is_enabled;
  bool is_fixed;
  mutex mtx;
  CommandBuffer* command_buffer;
private:
  vector<DrawElementsIndirectCommand> commands;

public:
  /**
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
    CommandBuffer* command_buffer,
    bool is_enabled,
    int fixed_size = -1) :
    id{ id },
    shader{ shader },
    mode{ mode },
    is_enabled{ is_enabled },
    command_buffer{ command_buffer },
    is_fixed{ fixed_size > 0 } {
    if (fixed_size > 0) {
      commands.resize(fixed_size);
    }
  }
  const unsigned int id;
  // caller must aquire it on write
  GLenum mode;  // GL_TRIANGLES GL_LINES
  Shader* shader;

  unsigned int GetCommandCount() {
    return commands.size();
  }

  void AddCommand(MeshDataSlots& mesh_slots) {
    lock_guard l(mtx);

    auto command = CreateCommand(mesh_slots);
    commands.push_back(command);
    is_dirty = true;
  }

  void PutCommand(MeshDataSlots& mesh_slots, unsigned int index) {
    assert(is_fixed);
    lock_guard l(mtx);

    auto command = CreateCommand(mesh_slots);
    commands[index] = command;
    is_dirty = true;
  }

  void PreDraw() {
    if (is_dirty) {
      BufferFrameCommands();
    }
  }
  void PostDraw() {
    Reset();
  }
private:
  inline DrawElementsIndirectCommand CreateCommand(MeshDataSlots& mesh_slots) {
    assert(mesh_slots.num_instances > 0);
    //assert(!mesh_slots.IsBufferIdAllocated());
    assert(mesh_slots.index_slot.IsSlotAllocated());
    assert(mesh_slots.vertex_slot.IsSlotAllocated());
    DrawElementsIndirectCommand command;
    mesh_slots.buffer_id = commands.size();
    command.instanceCount = mesh_slots.num_instances;
    command.count = mesh_slots.index_slot.used;
    command.firstIndex = mesh_slots.index_slot.offset;
    command.baseVertex = mesh_slots.vertex_slot.offset;
    command.baseInstance = mesh_slots.buffer_id;
    return command;
  }
  inline void BufferFrameCommands() {
    if (is_dirty) {
      CommandBuffersManager::GetInstance()->BufferCommands(command_buffer, commands, 0);
      is_dirty = false;
    }
  }
  inline void Reset() {
    if (!is_fixed) {
      commands.clear();
    }
  }
};
