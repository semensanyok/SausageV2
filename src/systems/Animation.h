#pragma once
#include "sausage.h"
#include "Structures.h"
#include "MeshManager.h"
#include "utils/AssimpHelper.h"

using namespace std;
using namespace glm;

struct FinalTransform {
    mat4 trans;
    mat4 parent;
    Bone* bone;
};

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
    void QueueMeshAnimUpdate(MeshData* mesh) {
        active_anims[mesh->id] = mesh;
    }
    void PlayAnim() {
        auto active_anim = active_anims.begin();
        while (active_anim != active_anims.end()) {
            if (active_anim->second->active_animations.empty()) {
                active_anim = active_anims.erase(active_anim);
                continue;
            }
            map<unsigned int, mat4> final_transforms;
            auto mesh = active_anim->second;
            for (auto& anim : mesh->active_animations) {
                uint32_t current_time = state_manager->seconds_since_start - anim.start_time;
                // FBX time, TODO: dae/gltf time.
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

                    vec3 bone_scale;
                    quat bone_rotation;
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
                            //bone_translate = mix(cur.second, (*next).second, blend);
                            bone_translate = cur.second;
                            break;
                        }
                    }
                    while (time_rotate != frame.second.time_rotation.end()) {
                        auto& cur = *time_rotate;
                        auto& next = ++time_rotate;
                        if (next == frame.second.time_rotation.end()) {
                            bone_rotation = cur.second;
                            break;
                        }
                        if ((*next).first > current_time_ticks) {
                            float blend = (current_time_ticks - cur.first) / ((*next).first - cur.first);
                            //auto quat = mix(cur.second, (*next).second, blend);
                            //bone_rotation = mat4_cast(slerp(cur.second, (*next).second, blend));
                            bone_rotation = cur.second;
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
                    
                    auto anim_trans = translate(mat4(1), bone_translate);
                    // NOTE: use with FixFbxRotations enabled on load.//////////////////
                    // otherwise - fix by mesh->armature->transform for each bone each frame. /////
                    auto r_angle = degrees(angle(bone_rotation));
                    auto r_axis = axis(bone_rotation);
                    if (r_angle > 0.001) {
                        anim_trans = rotate(anim_trans, r_angle, r_axis);
                    }

                    
                    //auto anim_trans = rotate(mat4(1), radians((float)(SDL_GetTicks() /1000 % 360)), vec3(0,1,0)) * translate(mat4(1), vec3(1,1,1));
                    // if wasnt set as product of parents.
                    if (final_transforms.find(bone->id) == final_transforms.end()) {
                        final_transforms[bone->id] = anim_trans;
                    }
                    else {
                        final_transforms[bone->id] = anim_trans * final_transforms[bone->id];
                    }
                    for (auto& child : bone->children) {
                        SetTransformForHierarchy(child, anim_trans, final_transforms);
                    }
                }
            }
            for (auto& final_transform : final_transforms) {
                auto bone = mesh->armature->id_to_bone[final_transform.first];
            
                //final_transform.second = final_transform.second * bone->offset;
                //final_transform.second = bone->offset * final_transform.second;
                
                // FBX NOTE: mesh->armature->transform transforms each bone_rotation. (xzy to xyz or whatever.)
                // need to multiply each frame for each bone rotation.
                // better solution - use FixFbxRotations.
                // below most likely invalid. remove after testing.
                ////////////////////////////////////////////////////////////////////////////////////////////
                //final_transform.second = bone->offset * final_transform.second * mesh->armature->transform;
                //final_transform.second = mesh->armature->transform * final_transform.second * bone->offset;
            
                //final_transform.second = mesh->armature->transform * bone->offset * final_transform.second;
                //final_transform.second = final_transform.second * mesh->armature->transform * bone->offset;

                //final_transform.second = bone->offset * mesh->armature->transform * final_transform.second;
                //final_transform.second = final_transform.second * bone->offset * mesh->armature->transform;
                /////////////////////////////////////////////////////////////////////////////////////////////
            }
            state_manager->BufferBoneTransformUpdate(mesh, final_transforms);
            active_anim++;
        }
    }
    void SetTransformForHierarchy(
        Bone* child,
        mat4& parent_transform,
        map<unsigned int, mat4>& final_transforms) {
        if (final_transforms.find(child->id) == final_transforms.end()) {
            final_transforms[child->id] = mat4(1);
        }
        auto& final_transform = final_transforms[child->id];
        // assume child transform is not final but parent product when SetTransformForHierarchy called.
        // when anim transform calculated - no more calls, due to top-down animation node iteration.
        // hence the order, to multiply parents top-down.
        final_transform = parent_transform * final_transform;
        for (auto& child_of_child : child->children) {
            SetTransformForHierarchy(child_of_child, final_transform, final_transforms);
        }
    }
	Animation* CreateAnimation(string& anim_name, double duration, double ticks_per_seconds) {
		Animation * anim = new Animation{ anim_count++, anim_name, duration, ticks_per_seconds };
        anims.push_back(anim);
        return anim;
	}

    void LoadAnimationForMesh(
        const string& file_name,
        MeshData* mesh
    ) {
        bool is_dae = file_name.ends_with(".dae");
        bool is_gltf = file_name.ends_with(".glb") || file_name.ends_with(".gltf");
        bool is_fbx = file_name.ends_with(".fbx");
        
        if (mesh->armature == nullptr) {
            LOG((ostringstream()
                << "mesh '" << mesh->name
                << "' doesnt have armature. Skip LoadAnimationForMesh for file: "
                << file_name)
                .str());
            return;
        }
        ConfiguredAssmipImporter

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
                    auto bone_name = MeshManager::GetBoneName(channel->mNodeName.C_Str(), mesh->armature, is_dae);
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
                    if (is_fbx || is_dae) {
                        FixRotations(bone_frames.time_rotation);
                    }
                }
                if (mesh != nullptr) {
                    mesh->armature->name_to_anim[anim_name] = anim;
                }
            }
        }
    }

    void FixRotations(vector<pair<double, quat>>& bone_rotations) {
        auto first_rot_inversed = inverse(bone_rotations[0].second);
        auto rangle = angle(first_rot_inversed);
        auto raxis = axis(first_rot_inversed);

        for (auto& rot : bone_rotations) {
            rot.second = rotate(rot.second, rangle, raxis);
        }
    }
};