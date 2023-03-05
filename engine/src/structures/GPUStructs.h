#pragma once

#include "sausage.h"
#include "OpenGLHelpers.h"
#include "Interfaces.h"
#include "Macros.h"

using namespace std;
using namespace glm;

// --------------------------------------------------------------------------------------------------------------------
// Indirect structures start
struct DrawArraysIndirectCommand {
  GLuint count;
  GLuint primCount;
  GLuint first;
  GLuint baseInstance;
};
/**
 * @brief all instances managed via DrawCallManager
*/
struct DrawElementsIndirectCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint firstIndex;
  GLuint baseVertex;
  GLuint baseInstance;
  // if randomly initialized and used - hardware crash
  DrawElementsIndirectCommand()
    : count{ 0 },
    instanceCount{ 0 },
    firstIndex{ 0 },
    baseVertex{ 0 },
    baseInstance{ 0 } {};
};
// Indirect structures end


class Samplers : public SausageSystem {
  bool is_samplers_init = false;

public:
  GLuint basic_repeat = 0;
  GLuint font_sampler = 0;
  Samplers() {}
  void Init() {
    if (!is_samplers_init) {
      glCreateSamplers(1, &basic_repeat);
      glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      // glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      // glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      // glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER,
      // GL_LINEAR_MIPMAP_NEAREST); // bilinear
      glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER,
                          GL_LINEAR_MIPMAP_LINEAR);  // trilinear
      CheckGLError();

      glCreateSamplers(1, &font_sampler);
      glSamplerParameteri(font_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glSamplerParameteri(font_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glSamplerParameteri(font_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glSamplerParameteri(font_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      CheckGLError();
      is_samplers_init = true;
    }
  }
};

//// hash function for unordered map
//template<> struct std::hash<CommandBuffer> {
//  size_t operator()(CommandBuffer const& t) const {
//    return t.id;
//  }
//};
//
//// eq for hashmap/hashset
//bool operator==(const Shader& lhs, const Shader& rhs) {
//  return lhs.shader_id == rhs.shader_id;
//}
//// compare for map/set
//bool operator<(const Shader& lhs, const Shader& rhs) {
//  return lhs.shader_id < rhs.shader_id;
//}
