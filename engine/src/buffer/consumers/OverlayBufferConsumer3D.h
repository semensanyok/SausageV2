#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "BufferSettings.h"
#include "MeshDataTypes.h"

using namespace std;
using namespace glm;

class MeshManager;

class OverlayBufferConsumer3D : public MeshDataOverlay3DConsumerT {

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
};
