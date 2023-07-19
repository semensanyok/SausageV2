#pragma once

#include "sausage.h"
#include "GPUStructs.h"
#include "Macros.h"
#include "Vertex.h"
#include "MeshDataStruct.h"

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
  bool is_enabled;
  mutex mtx;
private:
  vector<DrawElementsIndirectCommand> commands = {};

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
    bool is_enabled) :
    id{ id },
    shader{ shader },
    mode{ mode },
    is_enabled{ is_enabled } {
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

    assert(mesh_slots.num_instances > 0);
    assert(!mesh_slots.IsBufferIdAllocated());
    assert(mesh_slots.index_slot.IsSlotAllocated());
    assert(mesh_slots.vertex_slot.IsSlotAllocated());
    DrawElementsIndirectCommand command;
    mesh_slots.buffer_id = commands.size();
    command.instanceCount = mesh_slots.num_instances;
    command.count = mesh_slots.index_slot.used;
    command.firstIndex = mesh_slots.index_slot.offset;
    command.baseVertex = mesh_slots.vertex_slot.offset;
    command.baseInstance = mesh_slots.buffer_id;

    commands.push_back(command);
  }
  inline void Reset() {
    lock_guard l(mtx);

    commands.clear();
  }
};
