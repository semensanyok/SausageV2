#include "MeshDataBufferConsumer.h"

using namespace std;

void MeshDataBufferConsumer::BufferMeshData(
    MeshDataBase* mesh,
    shared_ptr<MeshLoadData> load_data) {
  buffer->BufferMeshData(mesh, load_data);
}

void MeshDataBufferConsumer::ReleaseStorage(MeshDataBase* mesh) {
  buffer->ReleaseStorage(mesh);
}

void MeshDataBufferConsumer::Init() {
  BufferConsumer::Init();
}

void MeshDataBufferConsumer::Reset() {
}

void MeshDataBufferConsumer::BufferTransform(vector<MeshData*>& meshes) {
  buffer->BufferTransform(meshes);
}

void MeshDataBufferConsumer::BufferTransform(MeshData* mesh) {
  buffer->BufferTransform(mesh);
}

void MeshDataBufferConsumer::BufferLights(vector<Light*>& lights) {
  buffer->BufferLights(lights);
}

void MeshDataBufferConsumer::BufferMeshTexture(MeshData* mesh) {
  buffer->BufferMeshTexture(mesh);
}

void MeshDataBufferConsumer::BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
  buffer->BufferBoneTransform(bones_transforms);
}
