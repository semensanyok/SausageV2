#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "MeshManager.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"

class BulletDebugDrawerBufferConsumer : public BufferConsumer {
  unsigned long vertices_size = 1000;
  unsigned long indices_size = 1000;
  bool is_data_inited = false;
  MeshManager* mesh_manager;
  BufferStorage* buffer;
public:
  MeshData* mesh = nullptr;

  BulletDebugDrawerBufferConsumer(BufferStorage* buffer,
    MeshManager* mesh_manager) :
    BufferConsumer(buffer, BufferType::PHYSICS_DEBUG_BUFFERS),
    mesh_manager{ mesh_manager },
    buffer{ buffer }{
  };
  ~BulletDebugDrawerBufferConsumer() {
  };
  void BufferMeshData(vector<vec3>& vertices,
      vector<unsigned int>& indices,
      vector<vec3>& colors);
  void Init();
  void Reset();
};
