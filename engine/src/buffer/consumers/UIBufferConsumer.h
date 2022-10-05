#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "BufferSettings.h"
#include "Logging.h"
#include "MeshManager.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "GPUStructs.h"
#include "DrawCallManager.h"

using namespace std;
using namespace glm;

class UIBufferConsumer : public BufferConsumer {
  MeshManager* mesh_manager;

 public:
  UIBufferConsumer(BufferStorage* buffer,
    DrawCallManager* draw_call_manager,
    MeshManager* mesh_manager)
      : BufferConsumer(buffer, draw_call_manager, BufferType::UI_BUFFERS),
        mesh_manager{mesh_manager} {}
  ~UIBufferConsumer() {}
  // TODO:
  // track available buckets.
  void BufferMeshData(MeshDataUI* mesh,
                      vector<vec3>& vertices,
                      vector<unsigned int>& indices,
                      vector<vec3>& colors, vector<vec2>& uvs);
  void BufferTransform(MeshDataUI* mesh);
  void BufferSize(MeshDataUI* mesh, int min_x, int max_x, int min_y, int max_y);
  void Init();
  void Reset();

 private:
   void BufferFontTexture(MeshDataBase* mesh_data, Texture* texture);
};
