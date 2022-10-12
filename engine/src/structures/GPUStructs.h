#pragma once

#include "sausage.h"
#include "OpenGLHelpers.h"
#include "Interfaces.h"
#include "Macros.h"

using namespace std;
using namespace glm;

namespace BufferType {
  typedef int BufferTypeFlag;
  const BufferTypeFlag MESH_VAO = 1;
  const BufferTypeFlag VERTEX = 1 << 1;
  const BufferTypeFlag INDEX = 1 << 2;
  const BufferTypeFlag UNIFORMS = 1 << 3;
  const BufferTypeFlag TEXTURE = 1 << 4;
  const BufferTypeFlag LIGHT = 1 << 5;
  const BufferTypeFlag COMMAND = 1 << 6;

  const BufferTypeFlag FONT_TEXTURE = 1 << 7;
  const BufferTypeFlag UI_UNIFORMS = 1 << 8;
  const BufferTypeFlag CONTROLLER_UNIFORMS = 1 << 9;

  const BufferTypeFlag BLEND_TEXTURES_BY_MESH_ID = 1 << 10;
    
  // COMPOSITE FLAGS
  const BufferTypeFlag MESH_BUFFERS =
    MESH_VAO | VERTEX | INDEX | UNIFORMS | TEXTURE | LIGHT | COMMAND | BLEND_TEXTURES_BY_MESH_ID;
  const BufferTypeFlag PHYSICS_DEBUG_BUFFERS =
    MESH_VAO | VERTEX | INDEX | COMMAND;
  const BufferTypeFlag UI_BUFFERS =
    MESH_VAO | VERTEX | INDEX | COMMAND | TEXTURE | FONT_TEXTURE | UI_UNIFORMS | CONTROLLER_UNIFORMS;
  const BufferTypeFlag TERRAIN_BUFFERS =
    MESH_VAO | VERTEX | INDEX | LIGHT | COMMAND | TEXTURE | BLEND_TEXTURES_BY_MESH_ID;
};  // namespace BufferType

struct BufferLock {
  mutex data_mutex;
  condition_variable is_mapped_cv;
  bool is_mapped;
  inline void Wait(unique_lock<mutex>& data_lock) {
    is_mapped_cv.wait(data_lock);
  }
};

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

struct CommandBuffer {
  const GLuint id;
  DrawElementsIndirectCommand* ptr;
  // we have to use lock here
  // because command buffer must be "unmapped" before each drawcall
  BufferLock* buffer_lock;
  inline bool operator==(const CommandBuffer& other) { return id == other.id; }
  ~CommandBuffer() {
    delete buffer_lock;
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
