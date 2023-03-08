#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "MeshManager.h"
#include "GLVertexAttributes.h"

class MeshStaticBufferConsumer : public BufferConsumer<BlendTextures, MeshDataStatic, mat4, VertexStatic> {
public:
  MeshStaticBufferConsumer(
    BufferStorage* buffer,
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
  )
    : BufferConsumer(buffer, vertex_attributes, mesh_manager, BufferType::MESH_STATIC_BUFFERS) {
  };
  void BufferMeshData(MeshDataStatic* mesh,
    vector<vec3>& vertices,
    vector<unsigned int>& indices,
    vector<vec2>& uvs,
    vector<vec3>& normals) {
    auto load_data = mesh_manager->CreateLoadData<VertexStatic>(vertices, indices, normals, uvs);

    AllocateStorage(mesh->slots, vertices.size(), indices.size());
    BufferVertices(mesh->slots, load_data);
    if (mesh->textures.num_textures > 0) {
      BufferTexture(mesh, mesh->textures);
    }
    BufferTransform(mesh, mesh->transform);
  }
  void ReleaseSlots(MeshDataBase* mesh) override {
    buffer->ReleaseInstanceSlot<MeshDataStatic>(mesh->slots);
    vertex_attributes->ReleaseStorage<VertexStatic>(mesh->slots);
  }
};
