#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(mesh, load_data);
}

void MeshDataBufferConsumer::ReleaseInstanceSlot(MeshDataBase* mesh) {
  buffer->ReleaseInstanceSlot<MeshData>(mesh->slots);
}

void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}

void MeshDataBufferConsumer::BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
  buffer->BufferBoneTransform(bones_transforms);
}
