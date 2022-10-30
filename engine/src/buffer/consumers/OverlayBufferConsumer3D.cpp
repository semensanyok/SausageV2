#include "OverlayBufferConsumer3D.h"

void OverlayBufferConsumer3D::BufferMeshData(
  MeshDataOverlay3D* mesh,
  vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors,
  vector<vec2>& uvs,
  vector<vec3>& glyph_id,
  vec3 relative_position,
  Texture* font_data) {
  shared_ptr<MeshLoadData> load_data =
    mesh_manager->CreateLoadData(vertices, indices, colors, uvs, glyph_id);

  buffer->BufferMeshData(mesh, load_data);

  mesh->texture = font_data;
  mesh->texture->MakeResident();
  BufferFontTexture(mesh, mesh->texture);
  BufferTransform(mesh, mesh->transform);
}

void OverlayBufferConsumer3D::Init() {
  BufferConsumer::Init();
}
