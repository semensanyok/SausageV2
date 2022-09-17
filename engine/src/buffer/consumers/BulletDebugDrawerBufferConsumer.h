#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "MeshManager.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"

class BulletDebugDrawerBufferConsumer : public BufferConsumer {
    BufferMargins margins;
    unsigned long vertex_total = 0;
    unsigned long index_total = 0;
    unsigned long meshes_total = 1;
public:
    MeshData* mesh = nullptr;
    MeshManager* mesh_manager;
    BulletDebugDrawerBufferConsumer(BufferStorage* buffer, MeshManager* mesh_manager) : 
        BufferConsumer( buffer, BufferType::PHYSICS_DEBUG_BUFFERS), 
        mesh_manager{ mesh_manager } {
    };
    ~BulletDebugDrawerBufferConsumer() {
    };
    void BufferMeshData(vector<vec3>& vertices,
        vector<unsigned int>& indices,
        vector<vec3>& colors);
    void Init();
    void Reset();
};
