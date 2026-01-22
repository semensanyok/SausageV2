#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "Texture.h"
#include "Transform.h"
#include "InstanceSlot.h"

using namespace std;
using namespace glm;

class MeshDataUI : public MeshDataBase,
  public InstanceSlotUpdate,
  public BufferInstanceOffset,
  public Transform<vec2> {

  friend class MeshManager;

  typedef MeshDataUI MESH_TYPE;
public:
  vec2 transform;
  Texture* texture;
  int uniform_offset = -1;
private:
  MeshDataUI(unsigned long id, string name = "") :
    MeshDataUI(id, { 0, 0 }, nullptr, name) {};
  MeshDataUI(unsigned long id, vec2 transform, Texture* texture, string name = "") :
    MeshDataBase(id, name),
    Transform(transform),
    transform{ transform },
    texture{ texture } {};
  ~MeshDataUI() {};

public:

  inline unsigned int GetUniformOffset() override {
    return this->uniform_offset;
  }

  inline unsigned int GetInstanceOffset() override {
    return this->slots.buffer_id;
  }

  inline bool IsUniformOffsetAllocated() override {
    return this->uniform_offset > -1;
  }

  inline bool IsInstanceOffsetAllocated() override {
    return this->slots.IsBufferIdAllocated();
  }

  void IncNumInstancesSetInstanceId() override {
    // 1 inst atm
    this->slots.IncNumInstancesGetInstanceId();
  }
  /**
   * call once per frame for base mesh
  */
  void FinalizeCommandWithBuffer() override;
  /**
   * call once on first usage for new mesh
  */
  bool AllocateUniformOffset() override;

  void UnsetFrameOffsets() override {
    this->slots.UnsetBufferIdAndInstances();
  }

  void OnTransformUpdate() override;
private:
  virtual void SetUniformOffset(int uniform_offset) {
    this->uniform_offset = uniform_offset;
  };
};
