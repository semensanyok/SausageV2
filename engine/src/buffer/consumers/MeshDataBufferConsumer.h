#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Structures.h"
#include "Logging.h"
#include "OpenGLHelpers.h"
#include "BufferStorage.h"
#include "BufferConsumer.h"
#include "BufferSettings.h"
#include "MeshDataStruct.h"
#include "GPUUniformsStruct.h"
#include "MeshManager.h"

using namespace std;

class MeshDataBufferConsumer : public BufferConsumer<BlendTextures, MeshData, mat4, Vertex> {
public:
  MeshDataBufferConsumer(BufferStorage* buffer,
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager
  )
    : BufferConsumer(buffer, vertex_attributes, mesh_manager, BufferType::MESH_BUFFERS) {
  };
  ~MeshDataBufferConsumer() {
  };
  void BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms);
  void ReleaseSlots(MeshDataBase* mesh) override {
    buffer->ReleaseInstanceSlot<MeshData>(mesh->slots);
    vertex_attributes->ReleaseStorage<Vertex>(mesh->slots);
  }
};
