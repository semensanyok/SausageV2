#pragma once

#include "sausage.h"

using namespace glm;
using namespace std;

class BufferInstanceOffset {

public:
  virtual unsigned int GetUniformOffset() = 0;

  // used as offset to array, containing offset to buffer uniform arrays: transform, texture, ...
  // to not rebuffer data for instanced meshes (transform matrices) each time slot is reallocated.
  //    e.g. when we have 2 buffered instanced meshes, but then increased instances to 4.
  //    so we must buffer 4 subsequent data, which identifies given instance.
  //    we cannot rebuffer all buffered data to the end of each array.
  //    so, we rebuffer only subsequent indices to intance data arrays.
  virtual unsigned int GetInstanceOffset() = 0;

  virtual bool IsInstanceOffsetAllocated() = 0;

  virtual bool IsUniformOffsetAllocated() = 0;
};

class InstanceSlotUpdate : public SausageUserPointer {
  friend class BufferStorage;
public:
  virtual ~InstanceSlotUpdate() {};
  virtual void IncNumInstancesSetInstanceId() = 0;
  virtual bool AllocateUniformOffset() = 0;
  virtual void FinalizeCommandWithBuffer() = 0;
  virtual void UnsetFrameOffsets() = 0;
private:
  virtual void SetUniformOffset(int uniform_offset) = 0;
};
