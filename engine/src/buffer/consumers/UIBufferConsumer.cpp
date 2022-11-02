#include "UIBufferConsumer.h"

void UIBufferConsumer::BufferMeshData(MeshDataUI* mesh,
    vector<vec3>& vertices, vector<unsigned int>& indices, vector<vec3>& colors,
    vector<vec2>& uvs) {
  shared_ptr<MeshLoadData> load_data =
      mesh_manager->CreateLoadData(vertices, indices, colors, uvs);

  buffer->BufferMeshData(mesh->slots, load_data);

  if (mesh->texture) {
    mesh->texture->MakeResident();
    BufferTexture(*mesh, mesh->texture->id);
  }
  buffer->BufferTransform<vec2, MeshDataUI>(*mesh, mesh->transform);
}

void UIBufferConsumer::BufferSize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y) {
  buffer->BufferUniformDataUISize(mesh, min_x, max_x, min_y, max_y);
}

void UIBufferConsumer::Init() {
  BufferConsumer::Init();
}

void UIBufferConsumer::Reset() { Init(); }
