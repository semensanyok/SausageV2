#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "MeshDataTypes.h"
#include "BufferSettings.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "GPUStructs.h"

using namespace std;
using namespace glm;

class MeshManager;
class GLVertexAttributes;
class MeshDataUI;
class BufferInstanceOffset;

class UIBufferConsumer : public MeshDataUIConsumerT {
  MeshManager* mesh_manager;

 public:
  UIBufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
  )
    : BufferConsumer(vertex_attributes, mesh_manager, BufferType::UI_BUFFERS),
        mesh_manager{mesh_manager} {}
  ~UIBufferConsumer() {}
  void BufferMeshData(MeshDataUI* mesh,
                      vector<vec3>& vertices,
                      vector<unsigned int>& indices,
                      vector<vec3>& colors, vector<vec2>& uvs);
  // implemented in template
  //void BufferTransform(MeshDataUI* mesh);
  void BufferSize(BufferInstanceOffset* mesh, int min_x, int max_x, int min_y, int max_y);
  void Init();
  void Reset();
};
