#pragma once

#include "sausage.h"
#include "MeshManager.h"
#include "MeshDataTypes.h"
#include "GLVertexAttributes.h"
#include "BufferConsumer.h"

class MeshDataOutlineBufferConsumer : public MeshDataOutlineBufferConsumerT {
public:
    MeshDataOutlineBufferConsumer(
        GLVertexAttributes* vertex_attributes,
        MeshManager* mesh_manager
    )
        : BufferConsumer(vertex_attributes, mesh_manager, BufferType::OUTLINE_BUFFERS) {
    };
    ~MeshDataOutlineBufferConsumer() {
    };
};
