#include "OverlayBufferConsumer3D.h"
#include "MeshManager.h"

void OverlayBufferConsumer3D::BufferMeshData(
  MeshDataOverlay3D* mesh,
  vector<vec3>& vertices,
  vector<unsigned int>& indices,
  vector<vec3>& colors,
  vector<vec2>& uvs,
  vector<uint>& glyph_id,
  vec3 relative_position,
  Texture* font_data) {
  shared_ptr<MeshLoadData<VertexStatic>> load_data =
    mesh_manager->CreateLoadData<VertexStatic>(vertices, indices, colors, uvs, glyph_id);

  vertex_attributes->BufferVertices(mesh->slots, load_data);

  mesh->texture = font_data;
  mesh->texture->MakeResident();
  BufferTexture(mesh, mesh->texture->id);
  BufferTransform(mesh, mesh->transform);
}

void OverlayBufferConsumer3D::Init() {
  BufferConsumer::Init();
}
