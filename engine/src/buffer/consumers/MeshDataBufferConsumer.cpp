#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(mesh, load_data);
}

void MeshDataBufferConsumer::ReleaseInstanceSlot(MeshDataBase* mesh) {
  buffer->ReleaseInstanceSlot(mesh->slots);
}

void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

void MeshDataBufferConsumer::BufferTransform(BufferInstanceOffset* offset, mat4& transform) {
  buffer->BufferTransform(offset, transform);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}

void MeshDataBufferConsumer::BufferTexture(BufferInstanceOffset* offset, Texture* texture) {
  buffer->BufferTexture(offset, texture);
}

void MeshDataBufferConsumer::BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
  buffer->BufferBoneTransform(bones_transforms);
}
