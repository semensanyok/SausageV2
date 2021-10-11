#pragma once

#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "MeshManager.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "sausage.h"
using namespace std;
using namespace glm;

class FontBufferConsumer : public BufferConsumer {
  MeshManager* mesh_manager;
  BufferMargins margins;
  unsigned long vertex_total = 0;
  unsigned long index_total = 0;
  unsigned long meshes_total = 0;

 public:
  FontBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager)
      : BufferConsumer(buffer, BufferType::FONT_BUFFERS),
        mesh_manager{mesh_manager} {}
  ~FontBufferConsumer() {}
  // TODO:
  // track available buckets.
  void BufferMeshDataUI(MeshDataFontUI* mesh,
                        vector<vec3>& vertices,
                        vector<unsigned int>& indices,
                        vector<vec3>& colors, vector<vec2>& uvs,
                        vec3 relative_position,
                        Texture* font_data);
  void BufferMeshData3D(MeshDataFont3D* mesh,
                        vector<vec3>& vertices,
                        vector<unsigned int>& indices,
                        vector<vec3>& colors,
                        vector<vec2>& uvs,
                        vector<vec3>& glyph_id,
                        vec3 relative_position,
                        Texture* font_data);

  void BufferTransform(MeshDataFontUI* mesh) {
    buffer->BufferUIFontTransform(mesh);
  }
  void BufferTransform(MeshDataFont3D* mesh) {
    buffer->Buffer3DFontTransform(mesh);
  }
  void Init() {
    margins = buffer->RequestStorage(BufferSettings::Margins::FONT_VERTEX_PART,
                                     BufferSettings::Margins::FONT_INDEX_PART);
    vertex_total = margins.start_vertex;
    index_total = margins.start_index;
  }
  void Reset() { Init(); }

 private:
  void BufferFontTexture(MeshDataBase* mesh_data, Texture* texture) {
    buffer->BufferFontTexture(mesh_data, texture);
  }
};
