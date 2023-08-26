#pragma once

#include "sausage.h"
//#include "MeshDataStruct.h"
#include "PhysicsStruct.h"
#include "Interfaces.h"
#include "Spatial.h"
#include "BoundingVolume.h"
#include "InstanceSlot.h"
#include "Transform.h"
#include "BufferStorage.h"
#include "DrawCall.h"

/**
 * instance of MeshData
 * (same vertices + textures)
*/
template <typename MESH_TYPE, typename UNIFORM_DATA_TYPE>
class MeshDataInstance :
  public BufferInstanceOffset,
  public InstanceSlotUpdate,
  public SausageUserPointer,
  public Spatial,
  public Transform<mat4>
{
  friend class MeshManager;

  friend class BufferStorage;
private:
  // `offset` into transform/texture array
  // const, allocated on instance creation
  int uniform_offset = -1;
  int instance_id = -1;

public:
  MeshDataInstance(mat4& transform, MESH_TYPE* base_mesh,
    BoundingBox* bv) :
    instance_id{ -1 }, base_mesh{ base_mesh },
    uniform_offset{ -1 },
    name{ format("{}_{}", base_mesh->name, to_string(instance_id)) },
    // 1. controversial, bcs limits MESH_TYPE variance to Spatial
    Transform(transform),
    Spatial(bv, transform) {
  }
  string name;
  PhysicsData* physics_data;

  MESH_TYPE* base_mesh;

  inline unsigned int GetUniformOffset() override {
    return this->uniform_offset;
  }

  inline unsigned int GetInstanceOffset() override {
    return this->base_mesh->slots.buffer_id + instance_id;
  }

  inline bool IsUniformOffsetAllocated() override {
    return this->uniform_offset > -1;
  }

  inline bool IsInstanceOffsetAllocated() override {
    return base_mesh->slots.IsBufferIdAllocated()
      && instance_id > -1;
  }
  
  void IncNumInstancesSetInstanceId() override {
    if (IsInstanceOffsetAllocated()) {
      return;
    }
    instance_id = this->base_mesh->slots.IncNumInstancesGetInstanceId();
  }
  void UnsetFrameOffsets() override {
    this->base_mesh->slots.UnsetBufferIdAndInstances();
    instance_id = -1;
  }

  void FinalizeCommandWithBuffer() {
    if (uniform_offset > 0) {
      // base_mesh responsible for adding draw command once per frame, skipping subsequent calls. ugly.
      base_mesh->FinalizeCommandWithBuffer();
      DrawCall* dc = this->base_mesh->dc;
      BufferStorage::GetInstance()->BufferUniformOffset<MESH_TYPE>(this);
    }
  }

  /**
  * call once on first usage for new mesh
  */
  bool AllocateUniformOffset() {
    if (IsUniformOffsetAllocated()) {
      return true;
    }
    MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>* p = this;
    return BufferStorage::GetInstance()->AllocateUniformOffset<UNIFORM_DATA_TYPE, MESH_TYPE>(p);
  }

  void OnTransformUpdate() {
    // buffer must be called each frame, because instance slot is new each time?
    // no. only instance slot update each frame.
    // transform update only if dirty
    if (Transform::is_dirty) {
      bv->Transform(ReadTransform());
      BufferStorage::GetInstance()->BufferTransform<mat4, MESH_TYPE>(this, ReadTransform());
      Transform::is_dirty = false;
    }
  }

private:
  virtual void SetUniformOffset(int uniform_offset) {
    this->uniform_offset = uniform_offset;
  };
  bool BufferUniformOffset();
  ~MeshDataInstance() {
    BufferStorage::GetInstance()->ReleaseUniformOffset<UNIFORM_DATA_TYPE, MESH_TYPE>(this);
  }
};
