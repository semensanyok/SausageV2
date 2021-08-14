#pragma once

#include "../../sausage.h"
#include "../../Settings.h"
#include "../../Structures.h"
#include "../../Logging.h"
#include "../../OpenGLHelpers.h"
#include "../BufferStorage.h"

; // TODO: fix. find in previous imports?

using namespace std;

class MeshDataBufferConsumer {
    BufferMargins margins;
    BufferStorage* buffer;
    unsigned long vertex_total = 0;
    unsigned long index_total = 0;
    unsigned long meshes_total = 0;
public:
    MeshDataBufferConsumer(BufferStorage* buffer) : buffer{ buffer } {
    };
    ~MeshDataBufferConsumer() {
    };
    void BufferMeshData(vector<shared_ptr<MeshLoadData>>& load_data, bool is_transform_used = true);
    void Init();
    void Reset();
    void SetBaseMeshForInstancedCommand(vector<shared_ptr<MeshLoadData>>& new_meshes);
};