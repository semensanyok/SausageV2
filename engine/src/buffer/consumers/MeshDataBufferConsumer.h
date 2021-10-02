#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"

using namespace std;

class MeshDataBufferConsumer : BufferConsumer {
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
    void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data, bool is_transform_used = true);
    void Init();
    void Reset();
    void SetBaseMeshForInstancedCommand(vector<shared_ptr<MeshLoadData>>& new_meshes);
    void BufferTransform(vector<MeshData*>& meshes);
    void BufferLights(vector<Light*>& lights);
};