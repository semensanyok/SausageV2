#pragma once

#include "sausage.h"
#include "Vertex.h"
#include "Structures.h"
#include "GPUStructs.h"
#include "GPUUniformsStruct.h"
#include "Interfaces.h"
#include "TextureStruct.h"
#include "BufferSettings.h"
#include "Arena.h"

using namespace std;
using namespace glm;

class   Texture;
class   Armature;
class   PhysicsData;

struct MeshLoadData {
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  MaterialTexNames* tex_names = nullptr;
  Armature* armature = nullptr;
  PhysicsData* physics_data = nullptr;
  string name;
  mat4 transform;
};

class MeshDataBase {
public:
  unsigned long id;
  // in glsl == gl_BaseInstanceARB
  long buffer_id;
  // in glsl == gl_InstanceID
  unsigned long instance_id;
  // Buffer offsets /////////////////////
  MemorySlot vertex_slot;
  MemorySlot index_slot;
  int transform_offset;
  //    commands are now managed via DrawCallManager
  //    instance_count set in DrawElementsIndirectCommand. dont duplicate counter here.
  //    (refer to DrawCallManager->...command to get instance count)
  // DrawElementsIndirectCommand command;
  // long instance_count;
  long command_offset;
  //////////////////////////////////////
  // for instanced meshes not allocated vertex/index offsets(?)
  MeshDataBase* base_mesh;
  MeshDataBase()
    : vertex_slot{ Arena::NULL_SLOT },
    index_slot{ Arena::NULL_SLOT },
    instance_id{ 0 },
    buffer_id{ -1 },
    base_mesh{ nullptr },
    transform_offset{ -1 },
    command_offset{ -1 }
    //instance_count{ 1 }
  {};
  virtual ~MeshDataBase() {};
};

class MeshData : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
public:
  mat4 transform;
  bool is_transparent;

  string name;
  BlendTextures textures;
  // _t_odo make reusable
  Armature* armature;
  // _t_odo make reusable
  PhysicsData* physics_data;

private:
  MeshData()
    : textures{ {0, {}} },
    physics_data{ nullptr },
    is_transparent{ false },
    transform{ mat4(1.0) } {};
  MeshData(MeshLoadData* load_data)
    : textures{ {0, {}} },
    physics_data{ load_data->physics_data },
    armature{ load_data->armature },
    name{ load_data->name },
    transform{ load_data->transform },
    is_transparent{ false } {};
  ~MeshData() {
  };
};

class MeshDataClickable : public SausageUserPointer {
public:
  MeshData* mesh_data;
  MeshDataClickable(MeshData* mesh_data) : mesh_data{ mesh_data } {};
  void Call() {
    cout << "RayHit from mesh " << mesh_data->name << endl;
  }
};

class MeshDataUI : public MeshDataBase {
  friend class MeshManager;
public:
  vec2 transform;
  Texture* texture;
private:
  MeshDataUI() : texture{ nullptr } {};
  MeshDataUI(vec2 transform, Texture* texture) :
    transform{ transform },
    texture{ texture } {};
  ~MeshDataUI() {};
};

class MeshDataOverlay3D : public MeshDataBase {
  friend class MeshManager;
public:
  string text;
  mat4 transform;
  Texture* texture;
private:
  MeshDataOverlay3D(string& text, mat4& transform) : text{ text }, transform{ transform }, texture{ nullptr } {};
  MeshDataOverlay3D() : texture{ nullptr } {};
  ~MeshDataOverlay3D() {};
};
