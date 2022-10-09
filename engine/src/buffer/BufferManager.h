#pragma once

#include "sausage.h"
#include "Structures.h"
#include "MeshDataBufferConsumer.h"	
#include "BufferStorage.h"
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"

class BufferManager : public SausageSystem {
  MeshManager* mesh_manager;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer;
public:
  BufferStorage* storage;

  MeshDataBufferConsumer* mesh_data_buffer;
  UIBufferConsumer* ui_buffer;
  OverlayBufferConsumer3D* overlay_3d_buffer;

  BufferManager(MeshManager* mesh_manager) :
    mesh_manager{ mesh_manager } {};
  void Init();
  void Reset() { Init(); };
  BulletDebugDrawerBufferConsumer* GetPhysDebugDrawer();
  //void _DisposeBuffer() {
  //	renderer->RemoveBuffer(buffer);
  //	buffer->Dispose();
  //	delete buffer;
  //}
};
