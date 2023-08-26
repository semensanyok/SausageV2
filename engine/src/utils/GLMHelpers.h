#pragma once

#include "sausage.h"
using namespace std;
using namespace glm;

bool IsIdentity(mat4& m1);

inline std::ostream& operator<<(std::ostream& os, vec3 const& v) {
  return os << '{' << v[0] << v[1] << v[2] << '}';
}
