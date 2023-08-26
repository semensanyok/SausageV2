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
#include "BoundingVolume.h"

using namespace std;
using namespace glm;

class   Texture;
class   Armature;
class   PhysicsData;
class   DrawCall;

struct MeshLoadDataBase {
  Armature* armature = nullptr;
  BoundingBox* bv = nullptr;
  string name;
  mat4 transform;
};

template <typename VERTEX_TYPE>
struct MeshLoadData : public MeshLoadDataBase {
  vector<VERTEX_TYPE> vertices;
  vector<unsigned int> indices;
};

class MeshDataSlots {
public:
  // - count == 1 for single mesh. multiple for instanced meshes
  // - used as offset to arrays: transform, texture, ...?
  // - can be reallocated when num of instanced meshes exceeds
  // Buffer offsets /////////////////////
  MemorySlot vertex_slot;
  MemorySlot index_slot;
  // in glsl == gl_BaseInstanceARB
  long buffer_id = -1;
  int num_instances = 0;
  MeshDataSlots() :
    vertex_slot { MemorySlots::NULL_SLOT },
    index_slot{ MemorySlots::NULL_SLOT },
    buffer_id{ -1 } {}
  inline bool IsBufferIdAllocated() {
    return buffer_id >= 0;
  }
  inline void UnsetBufferIdAndInstances() {
    buffer_id = -1;
    num_instances = 0;
  }
  unsigned int IncNumInstancesGetInstanceId() {
    return num_instances++;
  }
};

class MeshDataBase
{
  friend class MeshDataManager;
  friend class MeshManager;
  friend class MeshData;
  friend class MeshDataUI;
  friend class MeshDataOverlay3D;
  friend class MeshDataStatic;
  friend class MeshDataTerrain;
  friend class MeshDataOutline;

public:
  MeshDataSlots slots;
  // used for engine references. not used as index to GPU buffers.
  const unsigned long id;
  string name;
  DrawCall* dc;

  void FinalizeCommandWithBuffer();
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
  bool is_transparent;
  Armature* armature;
  //stored in MeshDataInstance
  //PhysicsData* physics_data;

private:
  MeshData(unsigned long id, string name = "")
    : MeshDataBase(id, name),
    is_transparent{ false } {};
  MeshData(unsigned long id, MeshLoadDataBase* load_data, string name = "")
    : MeshDataBase(id, name),
    armature{ load_data->armature },
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

  bool is_transparent;

  //stored in MeshDataInstance
  //PhysicsData* physics_data;

private:
  MeshDataStatic(unsigned long id, string name = "")
    : MeshDataStatic(id, nullptr, name) {};
  MeshDataStatic(unsigned long id, MeshLoadDataBase* load_data, string name = "")
    : MeshDataBase(id, name),
    is_transparent{ false } {};
  ~MeshDataStatic() {
  };
};

class MeshDataClickable : public SausageUserPointer {
public:
  string message;
  MeshDataClickable(string message) : message{ message } {};
  void Call() {
    cout << "RayHit from mesh " << message << endl;
  }
};
