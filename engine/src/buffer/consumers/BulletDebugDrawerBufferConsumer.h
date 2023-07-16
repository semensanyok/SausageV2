#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "MeshManager.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"
#include "MeshDataTypes.h"

// transform type mat4 not used. change to vec2 when needed.
class BulletDebugDrawerBufferConsumer : public MeshDataOutlineBufferConsumerT {
  unsigned long vertices_size = 1000;
  unsigned long indices_size = 1000;
  bool is_data_inited = false;
public:
  MeshDataOutline* mesh = nullptr;

  BulletDebugDrawerBufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
  )
    : BufferConsumer(vertex_attributes, mesh_manager, BufferType::OUTLINE_BUFFERS) {
  };
  ~BulletDebugDrawerBufferConsumer() {
  };
  void BufferMeshData(vector<vec3>& vertices,
      vector<unsigned int>& indices,
      vector<vec3>& colors);
  void Init();
  void Reset();
};
