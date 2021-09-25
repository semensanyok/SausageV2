#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "MeshManager.h"
#include "BufferStorage.h"

class BulletDebugDrawerBufferConsumer {
    BufferMargins margins;
    unsigned long vertex_total;
    unsigned long index_total;
    unsigned long meshes_total = 1;
public:
    MeshData* mesh = nullptr;
    MeshManager* mesh_manager;
    BufferStorage* buffer;
    inline BulletDebugDrawerBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager) : buffer{ buffer }, mesh_manager{ mesh_manager }{
    };
    inline ~BulletDebugDrawerBufferConsumer() {
    };
    void BufferMeshData(vector<vec3>& vertices,
        vector<unsigned int>& indices,
        vector<vec3>& colors,
        bool is_transform_used = false);
    void Init();
    void Reset();
};