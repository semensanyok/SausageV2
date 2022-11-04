#pragma once

#include "sausage.h"
#include "Structures.h"
#include "MeshManager.h"
#include "AssimpHelper.h"
#include "GLMHelpers.h"
#include "AnimHelpers.h"
#include "StateManager.h"
#include "BufferManager.h"
#include "ThreadSafeNumberPool.h"

using namespace std;
using namespace glm;

struct FinalTransform {
  mat4 trans;
  mat4 parent;
  Bone* bone;
};

class AnimationManager : public SausageSystem {
  vector<Animation*> anims;
  unordered_map<unsigned int, unsigned int> anims_num_of_active_instances;
  vector<ActiveAnimation*> all_anim_meshes;
  unordered_map<unsigned int, ActiveAnimation*> active_anims;
  ThreadSafeNumberPool* anim_id_pool;
  ThreadSafeNumberPool* active_anim_id_pool;
  StateManager* state_manager;
  MeshManager* mesh_manager;
  MeshDataBufferConsumer* mesh_data_buffer;
public:
  AnimationManager(StateManager* state_manager,
    MeshManager* mesh_manager,
    BufferManager* buffer_manager) :
    state_manager{ state_manager },
    mesh_manager{ mesh_manager },
    mesh_data_buffer{ buffer_manager->mesh_data_buffer },
    anim_id_pool{ new ThreadSafeNumberPool(MAX_MESHES_INSTANCES) },
    active_anim_id_pool{ new ThreadSafeNumberPool(MAX_MESHES_INSTANCES) }
  {};
  ~AnimationManager() {};
  void Reset();
  void QueueAnimUpdate(ActiveAnimation* active_anim);
  void PlayAnim();
  void SetTransformForHierarchy(
      const Armature* armature,
      const Bone* bone,
      const vector<ActiveAnimationBlend>& blend_anims,
      const mat4& parent_transform,
      unordered_map<unsigned int, mat4>& final_transforms,
      bool is_parent_anim = false);
  mat4 _GetBoneAnimation(const Bone* bone, const vector<ActiveAnimationBlend>& blend_anims, bool& out_is_bone_anim);
  Animation* CreateAnimation(string& anim_name, double duration, double ticks_per_seconds);
  bool DeleteAnimation(Animation* anim) {
    auto num_active_iter = anims_num_of_active_instances.find(anim->id);
    if (num_active_iter != anims_num_of_active_instances.end() && num_active_iter->second > 0) {
      DEBUG_EXPR(LOG(
        format("Skip deleting animation with id={} that have num of active instances={}",
          anim->id, num_active_iter->second)));
      return false;
    }
    anim_id_pool->ReleaseNumber(anim->id);
    delete anim;
    return true;
  }
  ActiveAnimation* CreateActiveAnimInstance(Armature* armature);
  void DeleteActiveAnimInstance(ActiveAnimation* active_anim) {
    for (auto& chanel_anim : active_anim->active_animations) {
      for (auto& anim : chanel_anim.second) {
        if (anims_num_of_active_instances.contains(anim.anim->id)) {
          anims_num_of_active_instances[anim.anim->id]--;
        }
      }
    }
    active_anim_id_pool->ReleaseNumber(active_anim->id);
    delete active_anim;
  }
  void LoadAnimationForArmature(
      const string& file_name,
      Armature* armature
  );
};
