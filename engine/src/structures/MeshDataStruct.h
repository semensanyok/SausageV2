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

struct MeshLoadDataBase {
  Armature* armature = nullptr;
  PhysicsData* physics_data = nullptr;
  string name;
  mat4 transform;
};

template <typename VERTEX_TYPE>
struct MeshLoadData : public MeshLoadDataBase {
  vector<VERTEX_TYPE> vertices;
  vector<unsigned int> indices;
};

// TODO: move parameters, accessed infrequently, to pointer structure. keep only instances slot as not pointer?
//       (to be more cpu cache friendly during processing many meshes)
class MeshDataSlots {
public:
  // - count == 1 for single mesh. multiple for instanced meshes
  // - used as offset to arrays: transform, texture, ...?
  // - can be reallocated when num of instanced meshes exceeds
  // Buffer offsets /////////////////////
  MemorySlot instances_slot;
  MemorySlot vertex_slot;
  MemorySlot index_slot;
  // in glsl == gl_BaseInstanceARB
  long buffer_id;
  MeshDataSlots() : instances_slot{ MemorySlots::NULL_SLOT },
    vertex_slot { MemorySlots::NULL_SLOT },
    index_slot{ MemorySlots::NULL_SLOT },
    buffer_id{ -1 } {}
  inline bool IsBufferIdAllocated() {
    return buffer_id >= 0;
  }
};

class MeshDataBase : public BufferInstanceOffset
{
  friend class MeshDataManager;
  friend class MeshManager;
  friend class MeshData;
  friend class MeshDataInstance;
  friend class MeshDataUI;
  friend class MeshDataOverlay3D;
  friend class MeshDataStatic;
  friend class MeshDataOutline;

public:
  MeshDataSlots slots;
  // used for engine references. not used as index to gl buffers.
  const unsigned long id;
  string name;
  inline unsigned long GetInstanceOffset() override {
    return slots.instances_slot.offset;
  }
  inline bool IsInstanceOffsetAllocated() override {
    return slots.instances_slot != MemorySlots::NULL_SLOT;
  }
private:
  MeshDataBase(unsigned long id, string name)
    : slots{ MeshDataSlots() }, id{ id }, name{ name.empty() ? to_string(id) : name}
  {};
  virtual ~MeshDataBase() {};
};

/**
 * base mesh with vertex/index/texture offsets pointers, shared among MeshDataInstanced
*/
class MeshData : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
public:

  mat4 transform;
  bool is_transparent;

  BlendTextures textures;
  // _t_odo make reusable
  Armature* armature;
  // _t_odo make reusable
  PhysicsData* physics_data;

private:
  MeshData(unsigned long id, string name = "")
    : MeshDataBase(id, name),
    textures{ {0, {}} },
    physics_data{ nullptr },
    is_transparent{ false },
    transform{ mat4(1.0) } {};
  MeshData(unsigned long id, MeshLoadDataBase* load_data, string name = "")
    : MeshDataBase(id, name), textures{ {0, {}} },
    physics_data{ load_data->physics_data },
    armature{ load_data->armature },
    transform{ load_data->transform },
    is_transparent{ false } {};
  ~MeshData() {
  };
};


/**
 * mesh with VertexStatic format, which doesnt contain armature bones (not animated).
 * name is a bit misleading. initially separated for VAO as mesh without armature.
 * however, it can move or be moved. in physics it can be dynamic body
*/
class MeshDataStatic : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
public:

  mat4 transform;
  bool is_transparent;

  BlendTextures textures;
  // _t_odo make reusable
  PhysicsData* physics_data;

private:
  MeshDataStatic(unsigned long id, string name = "")
    : MeshDataStatic(id, nullptr, name) {};
  MeshDataStatic(unsigned long id, MeshLoadDataBase* load_data, string name = "")
    : MeshDataBase(id, name), textures{ {0, {}} },
    physics_data{ load_data->physics_data },
    transform{ load_data->transform },
    is_transparent{ false } {};
  ~MeshDataStatic() {
  };
};
// $ TODO: ??? ____________---------------podnjkascASLDNSADFCKSDFJNWAOLEFNLWEFO-2

/**
 * instance of MeshData
 * (same vertices + textures)
*/
class MeshDataInstance : public BufferInstanceOffset, public SausageUserPointer {
  friend class MeshManager;
public:
  string name;
  mat4 transform;
  const long instance_id;
  // 1. base mesh can reallocate its command slot
  //    hence, updating all instances absolute offsets is cumbersome
  //    so we keep only relative instance_id, and dynamically get instance_offset
  //
  // 2. decided to have base pointer here. when need to reference to armature/physics/etc. - cast it to MeshData*
  const MeshDataBase* base_mesh;
  inline unsigned long GetInstanceOffset() override {
    return base_mesh->slots.instances_slot.offset + instance_id;
  }
  inline bool IsInstanceOffsetAllocated() override {
    return base_mesh->slots.instances_slot != MemorySlots::NULL_SLOT;
  }
private:
  MeshDataInstance(mat4& transform, long instance_id, MeshDataBase* base_mesh) :
    transform{ transform }, instance_id{ instance_id }, base_mesh{ base_mesh },
    name{ format("{}_{}", base_mesh->name, to_string(instance_id))} {
}
  ~MeshDataInstance() {};
};

class MeshDataUI : public MeshDataBase {
  friend class MeshManager;
public:
  vec2 transform;
  Texture* texture;
private:
  MeshDataUI(unsigned long id, string name = "") :
    MeshDataUI(id, { 0, 0 }, nullptr, name) {};
  MeshDataUI(unsigned long id, vec2 transform, Texture* texture, string name = "") :
    MeshDataBase(id, name),
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
  MeshDataOverlay3D(unsigned long id, mat4 transform, const char* text = nullptr, string name = "") :
    MeshDataBase(id, name),
    text{ text },
    transform{ transform },
    texture{ nullptr } {};
  ~MeshDataOverlay3D() {};
};


class MeshDataOutline : public MeshDataBase {
  friend class MeshManager;
public:
  //vec2 transform;
private:
  MeshDataOutline(unsigned long id, string name = "") :
    MeshDataBase(id, name) {};
  ~MeshDataOutline() {};
};

class MeshDataClickable : public SausageUserPointer {
public:
  string message;
  MeshDataClickable(string message) : message{ message } {};
  void Call() {
    cout << "RayHit from mesh " << message << endl;
  }
};

/*
this shader doesnt need instance slot
and only uses vertex/index arrays (doesnt use uniforms)
*/
class MeshDataPhysDebugDrawer {

};
