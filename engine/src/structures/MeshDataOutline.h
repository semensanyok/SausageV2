#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"

using namespace std;
using namespace glm;

class MeshDataOutline : public MeshDataBase, public SausageUserPointer {
  friend class MeshManager;
public:
  //vec2 transform;
private:
  MeshDataOutline(unsigned long id, string name = "") :
    MeshDataBase(id, name) {};
  ~MeshDataOutline() {};
};
