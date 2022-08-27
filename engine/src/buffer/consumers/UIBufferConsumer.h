#pragma once

#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "MeshManager.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "structures/Structures.h"
#include "structures/GPUStructs.h"

#include "sausage.h"
using namespace std;
using namespace glm;

class UIBufferConsumer : public BufferConsumer {
  MeshManager* mesh_manager;
  BufferMargins margins;
  unsigned long vertex_total = 0;
  unsigned long index_total = 0;
  unsigned long meshes_total = 0;

 public:
  UIBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager)
      : BufferConsumer(buffer, BufferType::UI_BUFFERS),
        mesh_manager{mesh_manager} {}
  ~UIBufferConsumer() {}
  // TODO:
  // track available buckets.
  void BufferMeshData(MeshDataUI* mesh,
                      vector<vec3>& vertices,
                      vector<unsigned int>& indices,
                      vector<vec3>& colors, vector<vec2>& uvs);
  void BufferTransform(MeshDataUI* mesh) {
    buffer->BufferUniformDataUITransform(mesh);
  }
  void BufferSize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y) {
    buffer->BufferUniformDataUISize(mesh, min_x, max_x, min_y, max_y);
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
