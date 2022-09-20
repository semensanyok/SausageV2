#include "Animation.h"

using namespace std;
using namespace glm;

void AnimationManager::Reset() {
    for (auto anim : anims) {
        delete anim;
    }
    for (auto anim_mesh : all_anim_meshes) {
        delete anim_mesh;
    }
    anims.clear();
    all_anim_meshes.clear();
    active_anims.clear();
    anim_count = 0;
}
void AnimationManager::QueueMeshAnimUpdate(AnimMesh* mesh) {
    active_anims[mesh->mesh->id] = mesh;
}
void AnimationManager::PlayAnim() {
    auto active_anim = active_anims.begin();
    while (active_anim != active_anims.end()) {
        if (active_anim->second->active_animations.empty()) {
            active_anim = active_anims.erase(active_anim);
            continue;
        }
        auto anim_mesh = active_anim->second;
        unordered_map<unsigned int, mat4> final_transforms;
        for (auto& blend_anims : anim_mesh->active_animations) {
            // FBX time, TODO: dae/gltf time.
            auto root_bone_parent = mat4(1);
            for (int i = 0; i < anim_mesh->mesh->armature->num_bones; i++) {
                auto root_bone = &anim_mesh->mesh->armature->bones[i];
                if (root_bone->parent == nullptr) {
                    SetTransformForHierarchy(anim_mesh->mesh, root_bone, blend_anims.second, root_bone_parent, final_transforms);
                }
            }
        }
        state_manager->BufferBoneTransformUpdate(final_transforms);
        active_anim++;
    }
}
void AnimationManager::SetTransformForHierarchy(
    MeshData* mesh,
    Bone* bone,
    vector<ActiveAnimation>& blend_anims,
    mat4& parent_transform,
    unordered_map<unsigned int, mat4>& final_transforms,
    bool is_parent_anim) {
    bool is_bone_anim = false;
    auto anim_trans = _GetBoneAnimation(bone, blend_anims, is_bone_anim);
    anim_trans = parent_transform * anim_trans;
    bool is_anim = is_bone_anim || is_parent_anim;
    if (is_anim) {
        final_transforms[bone->id] = mesh->armature->transform * anim_trans * bone->offset;
    }
    for (auto child_of_child : bone->children) {
        SetTransformForHierarchy(mesh, child_of_child, blend_anims, anim_trans, final_transforms, is_anim);
    }
}
mat4 AnimationManager::_GetBoneAnimation(Bone* bone, vector<ActiveAnimation>& blend_anims, bool& out_is_bone_anim) {

    vec3 bone_scale;
    quat bone_rotation;
    vec3 bone_translate;

    float last_anim_blend_weight = 0;

    for (auto& anim : blend_anims) {
        if (anim.anim->bone_frames.find(bone->name) == anim.anim->bone_frames.end()) {
            continue;
        }
        double current_time = state_manager->seconds_since_start - anim.start_time;
        double current_time_ticks = current_time * anim.anim->ticks_per_second;
        current_time_ticks = current_time_ticks > anim.anim->duration ? anim.anim->duration : current_time_ticks;

        auto& frame = anim.anim->bone_frames[bone->name];

        float blend_anims = 0;
        if (out_is_bone_anim) {
            blend_anims = anim.blend_weight / (last_anim_blend_weight + anim.blend_weight);
        }
        last_anim_blend_weight = std::max(anim.blend_weight, last_anim_blend_weight);
        bone_translate = _GetBlendAnim(frame.time_position, current_time_ticks, out_is_bone_anim, bone_translate, blend_anims);
        bone_rotation = _GetBlendAnim(frame.time_rotation, current_time_ticks, out_is_bone_anim, bone_rotation, blend_anims);
        bone_scale = _GetBlendAnim(frame.time_scale, current_time_ticks, out_is_bone_anim, bone_scale, blend_anims);
        out_is_bone_anim = true;
    }
    if (!out_is_bone_anim) {
        return bone->trans;
    }
    auto r_angle = angle(bone_rotation);
    auto r_axis = axis(bone_rotation);

    auto anim_trans = translate(mat4(1), bone_translate);
    anim_trans = rotate(anim_trans, r_angle, r_axis);
    anim_trans = scale(anim_trans, bone_scale);

    return anim_trans;
}
Animation* AnimationManager::CreateAnimation(string& anim_name, double duration, double ticks_per_seconds) {
    Animation* anim = new Animation{ anim_count++, anim_name, duration, ticks_per_seconds, {} };
    anims.push_back(anim);
    return anim;
}
AnimMesh* AnimationManager::CreateAnimMesh(MeshData* mesh) {
    auto anim_mesh = new AnimMesh(mesh);
    all_anim_meshes.push_back(anim_mesh);
    return anim_mesh;
}
void AnimationManager::LoadAnimationForMesh(
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
        Animation* anim = CreateAnimation(anim_name, aianim->mDuration, aianim->mTicksPerSecond);
        for (size_t j = 0; j < aianim->mNumChannels; j++)
        {
            auto channel = aianim->mChannels[j];
            auto bone_name = mesh_manager->GetBoneName(channel->mNodeName.C_Str(), mesh->armature, is_dae);
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
