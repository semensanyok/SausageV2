#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "MeshManager.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "BufferSettings.h"

using namespace std;
using namespace glm;

class OverlayBufferConsumer3D : public BufferConsumer<unsigned int, MeshDataOverlay3D, mat4, VertexStatic> {

public:
  OverlayBufferConsumer3D(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
  )
    : BufferConsumer(vertex_attributes, mesh_manager, BufferType::OUTLINE_BUFFERS) {}
  ~OverlayBufferConsumer3D() {}
  void BufferMeshData(MeshDataOverlay3D* mesh,
                      vector<vec3>& vertices,
                      vector<unsigned int>& indices,
                      vector<vec3>& colors,
                      vector<vec2>& uvs,
                      vector<uint>& glyph_id,
                      vec3 relative_position,
                      Texture* font_data);
  void Init();
  void Reset() { Init(); }
  void ReleaseSlots(MeshDataBase* mesh) {
    buffer->ReleaseInstanceSlot<MeshDataOverlay3D>(mesh->slots);
    vertex_attributes->ReleaseStorage<VertexStatic>(mesh->slots);
  }
};
