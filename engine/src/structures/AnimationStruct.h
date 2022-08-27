#pragma once

#include "../sausage.h"

using namespace std;
using namespace glm;

class MeshData;

struct Bone {
  unsigned int id;
  string name;
  /**
   * Matrix that transforms from bone space to mesh space in bind pose.
   */
  mat4 offset;
  mat4 trans;
  Bone* parent;
  vector<Bone*> children;
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
  Bone* bones;
  mat4 transform;
  map<string, Bone*> name_to_bone;
  map<unsigned int, Bone*> id_to_bone;
  map<string, Animation*> name_to_anim;
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
};  // namespace AnimBlendWeights

struct ActiveAnimation {
  double start_time;
  float blend_weight;
  Animation* anim;
};

class AnimMesh {
public:
  MeshData* mesh;
  map<AnimIndependentChannel, vector<ActiveAnimation>> active_animations;
  AnimMesh(MeshData* mesh) : mesh{ mesh } {};
  ~AnimMesh() {};
  inline void AddAnim(AnimIndependentChannel channel, Animation* anim,
                      float blend_weight = AnimBlendWeights::Weight1,
                      AnimBlendType blend_type = AnimBlendType::BLEND,
                      double start_time = 0) {
    switch (blend_type) {
    case REPLACE:
      active_animations[channel].clear();
      active_animations[channel].push_back({ start_time, blend_weight, anim });
      break;
    case BLEND:
      active_animations[channel].push_back({ start_time, blend_weight, anim });
      break;
    default:
      active_animations[channel].push_back({ start_time, blend_weight, anim });
      break;
    }
  }
};
