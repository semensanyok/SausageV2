#pragma once

#include "sausage.h"
#include "Settings.h"
#include "structures/Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"

using namespace std;

class MeshDataBufferConsumer : public BufferConsumer {
    BufferMargins margins;
    unsigned long vertex_total = 0;
    unsigned long index_total = 0;
    unsigned long meshes_total = 0;
public:
    MeshDataBufferConsumer(BufferStorage* buffer) : BufferConsumer(buffer, BufferType::MESH_BUFFERS) {
        unsigned long vertex_total = 0;
        unsigned long index_total = 0;
        unsigned long meshes_total = 0;
    };
    ~MeshDataBufferConsumer() {
    };
    void BufferMeshData(vector<MeshDataBase*>& load_data_meshes,
                        vector<shared_ptr<MeshLoadData>>& load_data);
    void Init();
    void Reset();
    void SetBaseMeshForInstancedCommand(
        vector<MeshDataBase*>& load_data_meshes,
        vector<shared_ptr<MeshLoadData>>& load_data);
    void BufferTransform(vector<MeshData*>& meshes);
    void BufferTransform(MeshData* mesh) {
      buffer->BufferTransform(mesh);
    }
    void BufferLights(vector<Light*>& lights);
    void BufferMeshTexture(MeshData* mesh) {
      buffer->BufferMeshTexture(mesh);
    };
    void BufferBoneTransform(map<unsigned int, mat4>& bones_transforms) {
      buffer->BufferBoneTransform(bones_transforms);
    };
};
