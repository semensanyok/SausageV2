#pragma once
#include "sausage.h"
#include "Structures.h"
#include "MeshManager.h"

using namespace std;
using namespace glm;

class AnimationManager {
	vector<Animation*> anims;
	map<unsigned int, MeshData*> active_anims;
	atomic<unsigned long> anim_count{ 0 };
    StateManager* state_manager;
public:
    AnimationManager(StateManager* state_manager) : state_manager{ state_manager } {};
	~AnimationManager() {};
    void Reset() {
        for (auto anim : anims) {
            delete anim;
        }
        anims.clear();
        active_anims.clear();
        anim_count = 0;
    }
    void StartAnim(MeshData* mesh) {
        active_anims[mesh->id] = mesh;
    }
    void PlayAnim() {
        auto active_anim = active_anims.begin();
        while (active_anim != active_anims.end()) {
            if (active_anim->second->active_animations.empty()) {
                active_anim = active_anims.erase(active_anim);
                continue;
            }
            map<unsigned int, Bone*> final_transforms;
            auto mesh = active_anim->second;
            for (auto& anim : mesh->active_animations) {
                uint32_t current_time = state_manager->seconds_since_start - anim.start_time;
                double current_time_ticks = current_time * anim.anim->ticks_per_second;
                current_time_ticks = current_time_ticks > anim.anim->duration ? anim.anim->duration : current_time_ticks;
                for (auto& frame : anim.anim->bone_frames) {
                    if (frame.first == mesh->armature->name) {
                        continue;
                    }
                    auto bone = mesh->armature->name_to_bone[frame.first];

                    auto time_scale = frame.second.time_scale.begin();
                    auto time_rotate = frame.second.time_rotation.begin();
                    auto time_position = frame.second.time_position.begin();
                    if (final_transforms.find(bone->id) == final_transforms.end()) {
                        final_transforms[bone->id] = bone;
                        bone->transform = mat4(1);
                    }
                    else {
                        cout << "inited";
                    }
                    vec3 bone_scale;
                    mat4 bone_rotation;
                    vec3 bone_translate;

                    while (time_position != frame.second.time_position.end()) {
                        auto& cur = *time_position;
                        auto& next = ++time_position;
                        if (next == frame.second.time_position.end()) {
                            bone_translate = cur.second;
                            break;
                        }
                        if ((*next).first > current_time_ticks) {
                            auto blend = (current_time_ticks - cur.first) / ((*next).first - cur.first);
                            bone_translate = mix(cur.second, (*next).second, blend);
                            //bone_translate = cur.second;
                            break;
                        }
                    }
                    while (time_rotate != frame.second.time_rotation.end()) {
                        auto& cur = *time_rotate;
                        auto& next = ++time_rotate;
                        if (next == frame.second.time_rotation.end()) {
                            bone_rotation = mat4_cast(cur.second);
                            break;
                        }
                        if ((*next).first > current_time_ticks) {
                            float blend = (current_time_ticks - cur.first) / ((*next).first - cur.first);
                            //auto quat = mix(cur.second, (*next).second, blend);
                            //bone_rotation = slerp(cur.second, (*next).second, blend);
                            bone_rotation = mat4_cast(cur.second);
                            break;
                        }
                    }
                    //while (time_scale != frame.second.time_scale.end()) {
                    //    auto& cur = *time_scale;
                    //    auto& next = ++time_scale;
                    //    if (next != frame.second.time_scale.end()) {
                    //        if ((*next).first > current_time_ticks) {
                    //            auto blend = (current_time_ticks - cur.first) / ((*next).first - cur.first);
                    //            //auto bone_scale = mix(cur.second, (*next).second, blend);
                    //            auto bone_scale = cur.second;
                    //            scale(bone->transform, bone_scale);
                    //        }
                    //    }
                    //    else {
                    //        scale(bone->transform, cur.second);
                    //    }
                    //}
                    bone->transform = translate(bone->transform, bone_translate);
                    //bone->transform = bone_rotation * bone->transform;
                    for (auto& child : bone->children) {
                        SetTransformForHierarchy(child, bone->transform, final_transforms);
                    }
                }
            }
            state_manager->AddBoneTransformUpdate(mesh);
            active_anim++;
        }
    }
    void SetTransformForHierarchy(Bone* bone, mat4& transform, map<unsigned int, Bone*>& final_transforms) {
        if (final_transforms.find(bone->id) == final_transforms.end()) {
            final_transforms[bone->id] = bone;
            bone->transform = mat4(1);
            bone->transform = transform * bone->transform;
        }
        else {
            bone->transform = transform * bone->transform;
        }
        for (auto& child : bone->children) {
            SetTransformForHierarchy(child, child->transform, final_transforms);
        }
    }
	Animation* CreateAnimation(string& anim_name, double duration, double ticks_per_seconds) {
		Animation * anim = new Animation{ anim_count++, anim_name, duration, ticks_per_seconds };
        anims.push_back(anim);
        return anim;
	}
    // note: 1 animation per file. (cant parse multiple anims via assimp)
    void LoadAnimationForMesh(
        const string& file_name,
        MeshData* mesh
    ) {
        if (mesh->armature == nullptr) {
            LOG((ostringstream()
                << "mesh '" << mesh->name
                << "' doesnt have armature. Skip LoadAnimationForMesh for file: "
                << file_name)
                .str());
            return;
        }
        Assimp::Importer assimp_importer;
        const aiScene* scene = assimp_importer.ReadFile(
            file_name, aiProcess_PopulateArmatureData);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            LOG((ostringstream() << "ERROR::ASSIMP:: " << string(assimp_importer.GetErrorString())).str());
            return;
        }

        for (size_t i = 0; i < scene->mNumAnimations; i++)
        {
            auto& aianim = scene->mAnimations[i];
            string anim_name = string(aianim->mName.C_Str());
            // blender exports name as ArmatureName|AnimName
            int delim_pos = anim_name.find("|");
            anim_name = anim_name.substr(delim_pos + 1, anim_name.size() - delim_pos);
            if (aianim->mNumChannels < 1) {
                continue;
            }
            if (aianim->mNumChannels > 1) {
                Animation* anim = CreateAnimation(anim_name, aianim->mDuration, aianim->mTicksPerSecond);
                for (size_t j = 0; j < aianim->mNumChannels; j++)
                {
                    auto channel = aianim->mChannels[j];
                    auto bone_name = string(channel->mNodeName.C_Str());
                    // bone at 0 index is armature name. P.S. Outdated
                    //if (j == 0) {
                    // continue;
                    //}
                    auto& bone_frames = anim->bone_frames[bone_name];
                    for (size_t k = 0; k < channel->mNumPositionKeys; k++)
                    {
                        auto& key = channel->mPositionKeys[k];
                        bone_frames.time_position.push_back({ key.mTime, FromAi(key.mValue) });
                    }
                    for (size_t k = 0; k < channel->mNumScalingKeys; k++)
                    {
                        auto& key = channel->mScalingKeys[k];
                        bone_frames.time_scale.push_back({ key.mTime, FromAi(key.mValue) });
                    }
                    for (size_t k = 0; k < channel->mNumRotationKeys; k++)
                    {
                        auto& key = channel->mRotationKeys[k];
                        bone_frames.time_rotation.push_back({ key.mTime, FromAi(key.mValue) });
                    }
                }
                if (mesh != nullptr) {
                    mesh->armature->name_to_anim[anim_name] = anim;
                }
            }
        }
    }
};