#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Interfaces.h"
#include "GLCommandBuffers.h"
#include "GLVertexAttributes.h"
#include "BufferStorage.h"
#include "MeshDataBufferConsumerShared.h"	
#include "MeshDataBufferConsumer.h"	
#include "MeshStaticBufferConsumer.h"
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"
#include "MeshTerrainBufferConsumer.h"
#include "MeshManager.h"
#include "TextureManager.h"
;
class BufferManager : public SausageSystem {
  inline static BufferManager* instance;

  MeshManager* mesh_manager = nullptr;
  BulletDebugDrawerBufferConsumer* bullet_debug_drawer_buffer = nullptr;
  // external systems
  TextureManager* texture_manager;
public:
  static BufferManager* GetInstance() {
    return instance;
  };
  // initialized via this class
  CommandBuffersManager* command_buffer_manager = nullptr;
  GLVertexAttributes* vertex_attributes = nullptr;
  MeshDataBufferConsumer* mesh_data_buffer = nullptr;
  UIBufferConsumer* ui_buffer = nullptr;
  OverlayBufferConsumer3D* overlay_3d_buffer = nullptr;
  MeshStaticBufferConsumer* mesh_static_buffer = nullptr;
  MeshTerrainBufferConsumer* mesh_terrain_buffer = nullptr;
  BufferStorage* buffer_storage = nullptr;

  BufferManager(MeshManager* mesh_manager,
    TextureManager* texture_manager) :
    mesh_manager{ mesh_manager },
    texture_manager{ texture_manager },
    buffer_storage{ BufferStorage::GetInstance() } {
    instance = this;
  };
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

  template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename VERTEX_TYPE>
  inline MeshDataBufferConsumerShared<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>* GetBuffer();
  template<>
  inline MeshDataBufferConsumerShared<BlendTextures, MeshData, Vertex>* GetBuffer() {
    return mesh_data_buffer;
  };
  template<>
  inline MeshDataBufferConsumerShared<BlendTextures, MeshDataStatic, VertexStatic>* GetBuffer() {
    return mesh_static_buffer;
  };

};
