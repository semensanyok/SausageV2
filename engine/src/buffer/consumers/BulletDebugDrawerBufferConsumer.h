#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "MeshManager.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"
#include "DrawCallManager.h"

class BulletDebugDrawerBufferConsumer : public BufferConsumer {
public:
    MeshData* mesh = nullptr;
    MeshManager* mesh_manager;
    BulletDebugDrawerBufferConsumer(BufferStorage* buffer,
      DrawCallManager* draw_call_manager,
      MeshManager* mesh_manager) :
        BufferConsumer( buffer, draw_call_manager, BufferType::PHYSICS_DEBUG_BUFFERS),
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
