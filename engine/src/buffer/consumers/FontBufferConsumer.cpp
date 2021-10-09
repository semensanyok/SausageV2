#include "FontBufferConsumer.h"

void FontBufferConsumer::BufferMeshDataUI(MeshDataFontUI* mesh,
    vector<vec3>& vertices, vector<unsigned int>& indices, vector<vec3>& colors,
    vector<vec2>& uvs, vec3 relative_position, Texture* font_data) {
  shared_ptr<MeshLoadData> load_data =
      mesh_manager->CreateMesh(vertices, indices, colors, uvs);

  buffer->BufferMeshData(mesh, load_data, vertex_total, index_total, meshes_total,
                         margins, SausageDefaults::DEFAULT_MESH_DATA_VECTOR);

  mesh->texture = font_data;
  mesh->texture->MakeResident();
  BufferFontTexture(mesh, mesh->texture);
  buffer->BufferUIFontTransform(mesh);
}
