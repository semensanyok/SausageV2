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
#include "TextureManager.h"
;
class BufferManager : public SausageSystem {
  MeshManager* mesh_manager = nullptr;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer = nullptr;
  // external systems
  TextureManager* texture_manager;
public:

  // initialized via this class
  CommandBuffersManager* command_buffer_manager = nullptr;
  GLVertexAttributes* vertex_attributes = nullptr;
  MeshDataBufferConsumer* mesh_data_buffer = nullptr;
  UIBufferConsumer* ui_buffer = nullptr;
  OverlayBufferConsumer3D* overlay_3d_buffer = nullptr;
  MeshStaticBufferConsumer* mesh_static_buffer = nullptr;
  BufferStorage* buffer_storage = nullptr;

  BufferManager(MeshManager* mesh_manager,
    TextureManager* texture_manager) :
    mesh_manager{ mesh_manager },
    texture_manager{ texture_manager },
    buffer_storage{ BufferStorage::GetInstance() } {};
  void Init();
  void Reset();
  BulletDebugDrawerBufferConsumer* GetPhysDebugDrawer();
  //void _DisposeBuffer() {
  //	renderer->RemoveBuffer(buffer);
  //	buffer->Dispose();
  //	delete buffer;
  //}

  void PreDraw() {
    buffer_storage->BindBuffers();
    command_buffer_manager->PreDraw();
  }

  void PostDraw() {
    command_buffer_manager->PostDraw();
  }
};
