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
#include "TextureStruct.h"
#include "TextureManager.h"
#include "MeshDataBufferConsumerShared.h"

using namespace std;

class MeshDataBufferConsumer : public MeshDataBufferConsumerShared<BlendTextures, MeshData, Vertex> {
public:
  MeshDataBufferConsumer(
    GLVertexAttributes* vertex_attributes,
    MeshManager* mesh_manager,
    TextureManager* texture_manager
  )
    : MeshDataBufferConsumerShared(vertex_attributes, mesh_manager, texture_manager, BufferType::MESH_BUFFERS) {
  };
  ~MeshDataBufferConsumer() {
  };
  void BufferBoneTransform(unordered_map<unsigned int, mat4>& bones_transforms) {
    buffer->BufferBoneTransform(bones_transforms);
  };
};
