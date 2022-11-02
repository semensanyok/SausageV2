#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"
#include "MeshDataStruct.h"
#include "GPUUniformsStruct.h"

using namespace std;

class MeshDataBufferConsumer : public BufferConsumer<BlendTextures, MeshData, mat4> {
public:
  MeshDataBufferConsumer(BufferStorage* buffer) :
    BufferConsumer(buffer, BufferType::MESH_BUFFERS) {
  };
  ~MeshDataBufferConsumer() {
  };
  void BufferMeshData(
    MeshDataSlots& slots,
    shared_ptr<MeshLoadData> load_data);
  void ReleaseInstanceSlot(MeshDataBase* mesh);
  void Init();
  void Reset();
  // implemented in base template
  //void BufferTransform(BufferInstanceOffset* offset, mat4& transform);
  void BufferLights(vector<Light*>& lights);
  // implemented in base template
  //void BufferTexture(BlendTextures& textures, BufferInstanceOffset& mesh);
  void BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms);
};
