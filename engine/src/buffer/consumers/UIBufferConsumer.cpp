#include "UIBufferConsumer.h"

void UIBufferConsumer::BufferMeshData(MeshDataUI* mesh,
    vector<vec3>& vertices, vector<unsigned int>& indices, vector<vec3>& colors,
    vector<vec2>& uvs) {
  shared_ptr<MeshLoadData<VertexUI>> load_data =
      mesh_manager->CreateLoadData<VertexUI>(vertices, indices, colors, uvs);

  AllocateStorage(mesh->slots, vertices.size(), indices.size());
  BufferVertices(mesh->slots, load_data);

  if (mesh->texture) {
    mesh->texture->MakeResident();
    BufferTexture(mesh, mesh->texture->id);
  }
  BufferTransform(mesh, mesh->transform);
}

void UIBufferConsumer::BufferSize(BufferInstanceOffset* mesh, int min_x, int max_x, int min_y, int max_y) {
  buffer->BufferUniformDataUISize(mesh, min_x, max_x, min_y, max_y);
}

void UIBufferConsumer::Init() {
  BufferConsumer::Init();
}

void UIBufferConsumer::Reset() { Init(); }
