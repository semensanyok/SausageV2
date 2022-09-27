#pragma once

#include "sausage.h"
#include "BufferConsumer.h"
#include "BufferStorage.h"
#include "Logging.h"
#include "MeshManager.h"
#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Structures.h"
#include "BufferSettings.h"

using namespace std;
using namespace glm;

class OverlayBufferConsumer3D : public BufferConsumer {
  MeshManager* mesh_manager;

 public:
  OverlayBufferConsumer3D(BufferStorage* buffer, MeshManager* mesh_manager)
      : BufferConsumer(buffer, BufferType::UI_BUFFERS),
        mesh_manager{mesh_manager} {}
  ~OverlayBufferConsumer3D() {}
  // TODO:
  // track available buckets.
  void BufferMeshData(MeshDataOverlay3D* mesh,
                        vector<vec3>& vertices,
                        vector<unsigned int>& indices,
                        vector<vec3>& colors,
                        vector<vec2>& uvs,
                        vector<vec3>& glyph_id,
                        vec3 relative_position,
                        Texture* font_data);
  void BufferTransform(MeshDataOverlay3D* mesh);
  void Init();
  void Reset() { Init(); }

 private:
   void BufferFontTexture(MeshDataBase* mesh_data, Texture* texture);
};
