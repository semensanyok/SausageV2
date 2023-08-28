#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Interfaces.h"
#include "GLVertexAttributes.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "MeshDataBufferConsumerShared.h"	
#include "MeshDataBufferConsumer.h"	
#include "MeshStaticBufferConsumer.h"
#include "BulletDebugDrawerBufferConsumer.h"
#include "UIBufferConsumer.h"
#include "OverlayBufferConsumer3D.h"
#include "MeshTerrainBufferConsumer.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "Vertex.h"
#include "MeshDataOutlineBufferConsumer.h"

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
  GLVertexAttributes* vertex_attributes = nullptr;
  MeshDataBufferConsumer* mesh_data_buffer = nullptr;
  UIBufferConsumer* ui_buffer = nullptr;
  OverlayBufferConsumer3D* overlay_3d_buffer = nullptr;
  MeshStaticBufferConsumer* mesh_static_buffer = nullptr;
  MeshTerrainBufferConsumer* mesh_terrain_buffer = nullptr;
  MeshDataOutlineBufferConsumer* mesh_outline_buffer = nullptr;
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
  }

  void PostDraw() {
  }

  template<typename MESH_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
  inline MeshDataBufferConsumerShared<BlendTextures, MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>* GetMeshDataBufferConsumer();
  template<>
  inline MeshDataBufferConsumerSharedT* GetMeshDataBufferConsumer() {
    return mesh_data_buffer;
  };
  template<>
  inline MeshStaticBufferConsumerSharedT* GetMeshDataBufferConsumer() {
    return mesh_static_buffer;
  };
  template<>
  inline MeshTerrainBufferConsumerSharedT* GetMeshDataBufferConsumer() {
    return mesh_terrain_buffer;
  };

  template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename TRANSFORM_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
  inline BufferConsumer<TEXTURE_ARRAY_TYPE, MESH_TYPE, TRANSFORM_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>* GetBufferConsumer();
  template<>
  inline MeshDataBufferConsumerT* GetBufferConsumer() {
    return mesh_data_buffer;
  };
  template<>
  inline MeshStaticBufferConsumerT* GetBufferConsumer() {
    return mesh_static_buffer;
  };
  template<>
  inline MeshTerrainBufferConsumerT* GetBufferConsumer() {
    return mesh_terrain_buffer;
  };

};
