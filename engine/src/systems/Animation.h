#pragma once

#include "sausage.h"
#include "Structures.h"
#include "MeshManager.h"
#include "AssimpHelper.h"
#include "GLMHelpers.h"
#include "AnimHelpers.h"
#include "StateManager.h"
#include "BufferManager.h"

using namespace std;
using namespace glm;

struct FinalTransform {
    mat4 trans;
    mat4 parent;
    Bone* bone;
};

class AnimationManager : public SausageSystem {
    vector<Animation*> anims;
    vector<AnimMesh*> all_anim_meshes;
    map<unsigned int, AnimMesh*> active_anims;
    atomic<unsigned long> anim_count{ 0 };
    StateManager* state_manager;
    MeshManager* mesh_manager;
    MeshDataBufferConsumer* mesh_data_buffer;
public:
    AnimationManager(StateManager* state_manager,
      MeshManager* mesh_manager,
      BufferManager* buffer_manager) :
      state_manager{ state_manager },
      mesh_manager{ mesh_manager },
      mesh_data_buffer{ buffer_manager->mesh_data_buffer } {};
    ~AnimationManager() {};
    void Reset();
    void QueueMeshAnimUpdate(AnimMesh* mesh);
    void PlayAnim();
    void SetTransformForHierarchy(
        MeshData* mesh,
        Bone* bone,
        vector<ActiveAnimation>& blend_anims,
        mat4& parent_transform,
        map<unsigned int, mat4>& final_transforms,
        bool is_parent_anim = false);
    mat4 _GetBoneAnimation(Bone* bone, vector<ActiveAnimation>& blend_anims, bool& out_is_bone_anim);
    Animation* CreateAnimation(string& anim_name, double duration, double ticks_per_seconds);
    AnimMesh* CreateAnimMesh(MeshData* mesh);
    void LoadAnimationForMesh(
        const string& file_name,
        MeshData* mesh
    );
};
