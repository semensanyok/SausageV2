#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(mesh, load_data);
}

void MeshDataBufferConsumer::ReleaseStorage(MeshDataBase* mesh) {
  buffer->ReleaseStorage(mesh->slots);
}

void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

void MeshDataBufferConsumer::BufferTransform(unsigned int instance_offset, mat4& transform) {
  buffer->BufferTransform(instance_offset, transform);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}

void MeshDataBufferConsumer::BufferTexture(unsigned int instance_offset, Texture* texture) {
  buffer->BufferTexture(instance_offset, texture);
}

void MeshDataBufferConsumer::BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
  buffer->BufferBoneTransform(bones_transforms);
}
