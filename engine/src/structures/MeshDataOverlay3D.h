#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "GPUUniformsStruct.h"
#include "InstanceSlot.h"

using namespace std;
using namespace glm;

class MeshDataOverlay3D : public MeshDataBase,
  public BufferInstanceOffset {
  friend class MeshManager;
public:
  string text;
  mat4 transform;
  Texture* texture;
  int uniform_offset = -1;
private:
  MeshDataOverlay3D(unsigned long id, mat4 transform, const char* text = nullptr, string name = "") :
    MeshDataBase(id, name),
    text{ text },
    transform{ transform },
    texture{ nullptr } {};
  ~MeshDataOverlay3D() {};

  inline unsigned int GetUniformOffset() override {
    return this->uniform_offset;
  }

  inline unsigned int GetInstanceOffset() override {
    return this->uniform_offset;
  }

  inline bool IsUniformOffsetAllocated() override {
    return this->uniform_offset > -1;
  }

  inline bool IsInstanceOffsetAllocated() override {
    return this->uniform_offset > -1;
  }
};
