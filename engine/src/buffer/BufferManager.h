#pragma once

#include "sausage.h"
#include "Structures.h"
#include "MeshDataBufferConsumer.h"	
#include "BufferStorage.h"
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"
#include "DrawCallManager.h"

class BufferManager : public SausageSystem {
  MeshManager* mesh_manager;
  DrawCallManager* draw_call_manager;

public:
  BufferStorage* storage;

  MeshDataBufferConsumer* mesh_data_buffer;
  UIBufferConsumer* ui_buffer;
  OverlayBufferConsumer3D* overlay_3d_buffer;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer;

  BufferManager(
    MeshManager* mesh_manager,
    DrawCallManager* draw_call_manager) :
    mesh_manager{ mesh_manager },
    draw_call_manager{ draw_call_manager } {};
  void Init();
  void Reset();
  //void _DisposeBuffer() {
  //	renderer->RemoveBuffer(buffer);
  //	buffer->Dispose();
  //	delete buffer;
  //}
};
