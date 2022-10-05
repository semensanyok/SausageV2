#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"
#include "DrawCallManager.h"

using namespace std;

class MeshDataBufferConsumer : public BufferConsumer {
public:
  MeshDataBufferConsumer(BufferStorage* buffer,
    DrawCallManager* draw_call_manager) :
    BufferConsumer(buffer, draw_call_manager, BufferType::MESH_BUFFERS) {
  };
  ~MeshDataBufferConsumer() {
  };
  bool BufferMeshData(
    DrawElementsIndirectCommand& command,
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data);
  void DeleteMesh(MeshDataBase* mesh) {
    buffer->ReleaseStorage(mesh);
    draw_call_manager->DisableCommand(mesh);
  }
  void Init();
  void Reset();
  void SetBaseMeshForInstancedCommand(
      vector<MeshDataBase*>& load_data_meshes,
      vector<shared_ptr<MeshLoadData>>& load_data);
  void BufferTransform(vector<MeshData*>& meshes);
  void BufferTransform(MeshData* mesh);
  void BufferLights(vector<Light*>& lights);
  void BufferMeshTexture(MeshData* mesh);
  void BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms);
};
