#pragma once

#include "sausage.h"
#include "Structures.h"
#include "BufferStorage.h"
#include "MeshDataBufferConsumer.h"	
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"

class BufferManager : public SausageSystem {
  MeshManager* mesh_manager = nullptr;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer = nullptr;
public:
  BufferStorage* storage = nullptr;

  MeshDataBufferConsumer* mesh_data_buffer = nullptr;
  UIBufferConsumer* ui_buffer = nullptr;
  OverlayBufferConsumer3D* overlay_3d_buffer = nullptr;

  BufferManager(MeshManager* mesh_manager) :
    mesh_manager{ mesh_manager } {};
  void Init();
  void Reset();
  BulletDebugDrawerBufferConsumer* GetPhysDebugDrawer();
  //void _DisposeBuffer() {
  //	renderer->RemoveBuffer(buffer);
  //	buffer->Dispose();
  //	delete buffer;
  //}
};
