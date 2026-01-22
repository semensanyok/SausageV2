#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "TextureStruct.h"
#include "TextureManager.h"
#include "GLVertexAttributes.h"
#include "GLBuffersCommon.h"
#include "MeshManager.h"
#include "Transform.h"

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
class MeshDataBufferConsumerShared : public BufferConsumer<TEXTURE_ARRAY_TYPE, MESH_TYPE, mat4, VERTEX_TYPE, UNIFORM_DATA_TYPE> {
  TextureManager* texture_manager;
  MeshManager* mesh_manager;
  GLVertexAttributes* vertex_attributes;
public:
  MeshDataBufferConsumerShared(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    TextureManager* texture_manager,
    BufferType::BufferTypeFlag buffer_type
  )
    : BufferConsumer<TEXTURE_ARRAY_TYPE, MESH_TYPE, mat4, VERTEX_TYPE, UNIFORM_DATA_TYPE>(vertex_attributes, mesh_manager, buffer_type),
    texture_manager{ texture_manager },
    mesh_manager{ mesh_manager },
    vertex_attributes{ vertex_attributes } {
  };
  ~MeshDataBufferConsumerShared() {
  };
  void ReleaseVertexStorage(MESH_TYPE* mesh) {
    vertex_attributes->ReleaseStorage<VERTEX_TYPE>(mesh->slots);
  }
  void ReleaseSlots(MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>* mesh) {
    BufferStorage::GetInstance()->ReleaseUniformOffset<UNIFORM_DATA_TYPE, MESH_TYPE>(mesh);
  }
  void BufferMeshData(MESH_TYPE* mesh,
    vector<vec3>& vertices,
    vector<unsigned int>& indices,
    vector<vec2>& uvs,
    vector<vec3>& normals) {
    auto load_data = mesh_manager->CreateLoadData<VERTEX_TYPE>(vertices, indices, normals, uvs);
    BufferMeshData(mesh, load_data);
  }
  void BufferMeshData(MESH_TYPE* mesh,
    vector<vec3>& vertices,
    vector<unsigned int>& indices,
    vector<vec2>& uvs,
    vector<vec3>& normals,
    vector<unsigned int>& uniform_id) {
    auto load_data = mesh_manager->CreateLoadData<VERTEX_TYPE>(vertices, indices, normals, uvs, uniform_id);
    BufferMeshData(mesh, load_data);
  }
  void BufferMeshData(MESH_TYPE* mesh,
    shared_ptr<MeshLoadData<VERTEX_TYPE>>& load_data) {
    this->BufferVertices(mesh->slots, load_data);
  }
  void BufferMeshDataInstance(MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>* mesh,
    BlendTextures& textures) {
    if (textures.num_textures > 0) {
      this->BufferTexture(mesh, textures);
    }
    mesh->OnTransformUpdate();
  }
};
