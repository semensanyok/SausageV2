#pragma once

#include "sausage.h"
#include "Settings.h"

using namespace std;
using namespace glm;
using namespace BufferSettings;

class BufferStorage;
class BufferConsumer;
class Shader;
class Texture;

struct Vertex {
  vec3 Position;
  vec3 Normal;
  vec2 TexCoords;
  vec3 Tangent;
  vec3 Bitangent;
  ivec4 BoneIds;
  vec4 BoneWeights;
};

// --------------------------------------------------------------------------------------------------------------------
// Indirect structures start
struct DrawArraysIndirectCommand {
  GLuint count;
  GLuint primCount;
  GLuint first;
  GLuint baseInstance;
};
struct DrawElementsIndirectCommand {
  GLuint count;
  GLuint instanceCount;
  GLuint firstIndex;
  GLuint baseVertex;
  GLuint baseInstance;
  // if randomly initialized and used - hardware crash
  DrawElementsIndirectCommand()
      : count{0}, instanceCount{0}, firstIndex{0}, baseVertex{0}, baseInstance{
                                                                      0} {};
};
// Indirect structures end
// --------------------------------------------------------------------------------------------------------------------
// --------------------------------------------------------------------------------------------------------------------
// Texture structures start

enum TextureType {
  Diffuse,  // sampler2DArray   index 0
  Normal,   //                        1
  Specular, //                        2 roughness in PBR
  AO,       //                        3
  Height,   //                        4 PBR
  Metal,    //                        5 PBR
            // Opacity, //                        6 use Diffuse alpha
};

struct MaterialTexNames {
  string diffuse;
  string normal;
  string specular;
  string height;
  string metal;
  string ao;
  string opacity;
  inline size_t Hash() {
    return hash<string>{}(string(diffuse)
                              .append(normal)
                              .append(specular)
                              .append(height)
                              .append(metal)
                              .append(ao)
                              .append(opacity));
  }
};
class Samplers {
  bool is_samplers_init = false;
public:
  GLuint basic_repeat = 0;
  GLuint font_sampler = 0;
  Samplers() {
  }
  void Init() {
      if (!is_samplers_init) {
          glCreateSamplers(1, &basic_repeat);
          glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_S, GL_REPEAT);
          glSamplerParameteri(basic_repeat, GL_TEXTURE_WRAP_T, GL_REPEAT);
          glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
          // glSamplerParameteri(basic_repeat, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
          //glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
          //glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); // bilinear
          glSamplerParameteri(basic_repeat, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // trilinear
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
// --------------------------------------------------------------------------------------------------------------------
// Texture structures end
enum LightType { Point, Directional, Spot };

namespace AttenuationConsts {
const float OGRE_P_L_ATT_DIST_7L = 0.7f;
const float OGRE_P_L_ATT_DIST_7Q = 1.8f;
const float OGRE_P_L_ATT_DIST_13L = 0.35f;
const float OGRE_P_L_ATT_DIST_13Q = 0.44f;
const float OGRE_P_L_ATT_DIST_20L = 0.22f;
const float OGRE_P_L_ATT_DIST_20Q = 0.20f;
} // namespace AttenuationConsts

struct Light {
  vec4 direction;
  vec4 position;
  vec4 color;
  vec4 specular;

  int type;
  float spot_inner_cone_cos;
  float spot_outer_cone_cos;
  float constant_attenuation;

  float linear_attenuation;
  float quadratic_attenuation;
  float padding[2] = {0, 0}; // OpenGL padding
};

struct Lights {
  int num_lights;
  float padding[3] = {0, 0, 0};
  Light lights[];
};

enum ShaderType { BlinnPhong, BRDF };

struct Bone {
  unsigned int id;
  string name;
  /**
   * Matrix that transforms from bone space to mesh space in bind pose.
   */
  mat4 offset;
  mat4 trans;
  Bone *parent;
  vector<Bone *> children;
};

struct BoneKeyFrames {
  vector<pair<double, vec3>> time_position;
  vector<pair<double, quat>> time_rotation;
  vector<pair<double, vec3>> time_scale;
};

struct Animation {
  unsigned int id;
  string name;
  double duration;
  double ticks_per_second;

  map<string, BoneKeyFrames> bone_frames;
};

struct Armature {
  string name;
  unsigned int num_bones;
  Bone *bones;
  mat4 transform;
  map<string, Bone *> name_to_bone;
  map<unsigned int, Bone *> id_to_bone;
  map<string, Animation *> name_to_anim;
};

// not blended channels
// animations, operating on different set of joints, can be applied
// independently
enum AnimIndependentChannel { CHANNEL1, CHANNEL2, CHANNEL3 };

enum AnimBlendType {
  REPLACE,
  BLEND,
};

namespace AnimBlendWeights {
inline float Weight1 = 1;
inline float Weight2 = 2;
inline float Weight10 = 10;
}; // namespace AnimBlendWeights

struct ActiveAnimation {
  double start_time;
  float blend_weight;
  Animation *anim;
};

// TODO: REFACTOR MESH GOD
//class Movable {
//    mat4 transform;
//
//};
//
//class Textured {
//public:
//    Texture* texture;
//    Textured() : texture{ nullptr } {};
//};

class MeshData {
public:
  unsigned long id;
  bool is_transparent;
  long buffer_id;
  unsigned long instance_id;
  unsigned long transform_offset;
  mat4 transform;
  string name;
  DrawElementsIndirectCommand command;
  BufferStorage *buffer;
  vec3 max_AABB;
  vec3 min_AABB;
  long vertex_offset;
  long index_offset;
  Texture *texture;

  MeshData *base_mesh;
  Armature *armature;

  MeshData()
      : vertex_offset{ -1 }, index_offset{ -1 }, buffer_id{ -1 }, transform_offset{0}, base_mesh{ nullptr },
        texture{nullptr}, armature{nullptr}, is_transparent{false} {};
};

class MeshDataFontUI : MeshData {

};
class MeshDataFont3D : MeshData {

};

class AnimMesh {
public:
  MeshData* mesh;
  map<AnimIndependentChannel, vector<ActiveAnimation>> active_animations;
  AnimMesh(MeshData* mesh) : mesh{ mesh } {};
  ~AnimMesh() {};
  inline void AddAnim(AnimIndependentChannel channel, Animation *anim,
               float blend_weight = AnimBlendWeights::Weight1,
               AnimBlendType blend_type = AnimBlendType::BLEND,
               double start_time = 0) {
    switch (blend_type) {
    case REPLACE:
      active_animations[channel].clear();
      active_animations[channel].push_back({start_time, blend_weight, anim});
      break;
    case BLEND:
      active_animations[channel].push_back({start_time, blend_weight, anim});
      break;
    default:
      active_animations[channel].push_back({start_time, blend_weight, anim});
      break;
    }
  }
};

struct MeshLoadData {
  MeshData *mesh;
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  MaterialTexNames tex_names;
  unsigned int instance_count;
  //~MeshLoadData() { cout << "MeshLoadData deleted: " << (mesh_data == nullptr
  //? "no mesh_data" : mesh_data->name) << endl; }
};

struct BufferLock {
  mutex data_mutex;
  condition_variable is_mapped_cv;
  bool is_mapped;
  inline void Wait(unique_lock<mutex> &data_lock) { is_mapped_cv.wait(data_lock); }
};

struct CommandBuffer {
  GLuint id;
  DrawElementsIndirectCommand *ptr;
  unsigned int size;
  BufferLock *buffer_lock;
  inline bool operator==(const CommandBuffer &other) { return id == other.id; }
};

struct DrawCall {
  int mode = GL_TRIANGLES; // GL_TRIANGLES GL_LINES
  BufferConsumer *buffer = nullptr;
  Shader *shader = nullptr;
  CommandBuffer *command_buffer;
  unsigned int command_count = 0;
  // custom data
  int num_lights = 0;
};

struct Shaders {
  Shader *blinn_phong;
  Shader *bullet_debug;
  Shader *stencil;
  Shader* font_ui;
  Shader* font_3d;
};

struct MeshUniformData {
  mat4 bones_transforms[MAX_BONES];
  mat4 transforms[MAX_TRANSFORM];
  unsigned int transform_offset[MAX_TRANSFORM_OFFSET];
};

struct FontUniformData {
    mat4 transforms[MAX_FONT_TRANSFORM];
    unsigned int transform_offset[MAX_FONT_TRANSFORM_OFFSET];
};

struct BufferMargins {
    unsigned long start_vertex;
    unsigned long end_vertex;
    unsigned long start_index;
    unsigned long end_index;
};

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
    const BufferTypeFlag FONT_UNIFORMS = 1 << 8;

    // COMPOSITE FLAGS
    const BufferTypeFlag MESH_BUFFERS = MESH_VAO | VERTEX | INDEX | UNIFORMS | TEXTURE | LIGHT | COMMAND;
    const BufferTypeFlag PHYSICS_DEBUG_BUFFERS = MESH_VAO | VERTEX | INDEX | COMMAND;
    const BufferTypeFlag FONT_BUFFERS = MESH_VAO | VERTEX | INDEX | COMMAND | FONT_TEXTURE | FONT_UNIFORMS;
};

namespace SausageDefaults {
    inline vector<MeshData*> DEFAULT_MESH_DATA_VECTOR;
};