#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Interfaces.h"
#include "GLCommandBuffers.h"
#include "GLVertexAttributes.h"
#include "BufferStorage.h"
#include "MeshDataBufferConsumer.h"	
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"
#include "MeshStaticBufferConsumer.h"
#include "MeshManager.h"
;
class BufferManager : public SausageSystem {
  MeshManager* mesh_manager = nullptr;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer = nullptr;
public:
  BufferStorage* storage = nullptr;
  CommandBuffersManager* command_buffer_manager;
  GLVertexAttributes* vertex_attributes;

  MeshDataBufferConsumer* mesh_data_buffer = nullptr;
  UIBufferConsumer* ui_buffer = nullptr;
  OverlayBufferConsumer3D* overlay_3d_buffer = nullptr;
  MeshStaticBufferConsumer* mesh_static_buffer = nullptr;

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
