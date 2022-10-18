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
  Armature* armature = nullptr;
  PhysicsData* physics_data = nullptr;
  string name;
  mat4 transform;
};

// TODO: move parameters, accessed infrequently, to pointer structure. keep only instances slot as not pointer?
//       (to be more cpu cache friendly during processing many meshes)
class MeshDataSlots {
public:
  // - count == 1 for single mesh. multiple for instanced meshes
  // - used as offset to arrays: transform, texture, ...?
  // - can be reallocated when num of instanced meshes exceeds
  MemorySlot instances_slot;
  // Buffer offsets /////////////////////
  MemorySlot vertex_slot;
  MemorySlot index_slot;
  // in glsl == gl_BaseInstanceARB
  long buffer_id;
  MeshDataSlots() : instances_slot{ MemorySlots::NULL_SLOT },
    vertex_slot { MemorySlots::NULL_SLOT },
    index_slot{ MemorySlots::NULL_SLOT },
    buffer_id{ -1 } {}
};

class MeshDataBase
{
public:
  MeshDataSlots slots;
  const unsigned long id;
  MeshDataBase()
    : vertex_slot{ Arena::NULL_SLOT },
    index_slot{ Arena::NULL_SLOT },
    // sign to have explicit initialization, to not accidentially write instance data to base mesh offset
    instance_id{ -1 },
    buffer_id{ -1 },
    instances_slot{ -1 },
    instance_offset{ 0 }
    //,instance_count{ 1 }
  {};
  virtual ~MeshDataBase() {};
};

struct MeshDataSlots {
  // count == 1 for single mesh. multiple for instanced meshes
  // used as offset to arrays: transform, texture, ...?
  MemorySlot instances_slot;
  // Buffer offsets /////////////////////
  MemorySlot vertex_slot;
  MemorySlot index_slot;
  // in glsl == gl_BaseInstanceARB
  long buffer_id;
};

/**
 * base mesh with vertex/index/texture offsets pointers, shared among MeshDataInstanced
*/
class MeshData : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
public:

  mat4 transform;
  bool is_transparent;

  string name;
  BlendTextures textures;
  // _t_odo make reusable (hash + search, like in TextureManager.cpp)
  Armature* armature;
  // _t_odo make reusable
  PhysicsData* physics_data;

private:
  MeshData()
    : MeshDataBase(), textures{ {0, {}} },
    physics_data{ nullptr },
    is_transparent{ false },
    transform{ mat4(1.0) } {};
  MeshData(MeshLoadData* load_data)
    : MeshDataBase(), textures{ {0, {}} },
    physics_data{ load_data->physics_data },
    armature{ load_data->armature },
    name{ load_data->name },
    transform{ load_data->transform },
    is_transparent{ false } {};
  ~MeshData() {
  };
};

// TODO: refer to this in storage buffer commands, migrate other systems
// to not store 100's null initialized pointers for each instance

/**
 * instance of MeshData
 * (same vertices + textures)
*/
class MeshDataInstance {
  mat4 transform;
  const long instance_id;
  // base mesh can reallocate its command slot
  // hence, updating all instances absolute offsets is cumbersome
  // so we keep only relative instance_id, and dynamically get instance_offset
  //const long instance_offset;
  const MeshData* base_mesh;
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
