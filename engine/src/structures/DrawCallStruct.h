#pragma once

#include "sausage.h"
#include "ShaderStruct.h"
#include "Arena.h"
#include "GPUStructs.h"
#include "MeshDataStruct.h"
#include "ThreadSafeNumberPool.h"
#include "Macros.h"
#include "GLBuffers.h"
#include "Vertex.h"
#include "GLCommandBuffers.h"

using namespace std;

/**
 * make sure to group VertexType in contigious series of draws
 */
enum class DrawOrder {
  //VertexType::MESH
  MESH,
  //VertexType::STATIC
  MESH_STATIC,
  OVERLAY_3D,
  //VERTEX_TYPE::UI
  UI_BACK,
  UI_TEXT,
  //VertexType::OUTLINE
  OUTLINE
};

// TODO: MESH DRAW HAS BOTH - STATIC AND ANIMATED (VertexType::MESH and VertexType::STATIC)
//       HANDLE IT SOMEHOW
inline VertexType GetVertexTypeByDrawOrder(DrawOrder draw_order) {
  switch (draw_order)
  {
  case DrawOrder::MESH:
    // TODO: MESH DRAW HAS BOTH - STATIC AND ANIMATED (VertexType::MESH and VertexType::STATIC)
//       HANDLE IT SOMEHOW
    return VertexType::MESH;
    break;
  case DrawOrder::OVERLAY_3D:
  case DrawOrder::MESH_STATIC:
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
// (referenced from base_instance_offset[buffer_id] -> instance_id to fetch textures/transforms)
// commented out because now it is used only in template function
//template<typename MeshDataClass>
class DrawCall {
  friend class DrawCallManager;
public:
  CommandBuffer* command_buffer;
  bool is_enabled;
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
    bool is_enabled) :
    id{ id },
    shader{ shader },
    mode{ mode },
    command_buffer{ command_buffer },
    is_enabled{ is_enabled } {
  }
  const unsigned int id;
  // caller must aquire it on write
  mutex mtx;
  GLenum mode;  // GL_TRIANGLES GL_LINES
  Shader* shader;

  unsigned int GetCommandCount() {
    return command_buffer->ptr->instances_slots.instances_slots.GetUsed();
  }
  unsigned int GetBaseOffset() {
    return command_buffer->ptr->instances_slots.instances_slots.GetBaseOffset();
  }

private:
  // allocates slot for single DrawElementsIndirectCommand
  void Allocate(MeshDataSlots& out_slots) {
    MemorySlot command_buffer_slot = command_buffer->ptr->instances_slots.Allocate(1);
    out_slots.buffer_id = command_buffer_slot.offset;
    assert(out_slots.IsBufferIdAllocated());
  }
  void Release(MeshDataSlots& out_slots) {
    MemorySlot to_release = { out_slots.buffer_id, 1 };
    command_buffer->ptr->instances_slots.Release(to_release);
  }
};
