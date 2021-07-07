#pragma once
#include "sausage.h"
#include "Structures.h"
#include "MeshManager.h"

using namespace std;

struct ActiveAnimation {
	double current_time;
	float blend_factor;
};
class AnimationManager {
public:
	AnimationManager() {};
	~AnimationManager() {};
	map<unsigned int, Animation*> anims;
	map<unsigned int, pair<Animation*, vector<MeshData*>>> active_anims;
	atomic<unsigned long> anim_count{ 0 };

	void PlayAnim(MeshData* mesh, Animation* animation, double anim_time) {
		//active_anims[animation->id].push_back(mesh);
	}
	Animation* CreateAnimation(string& anim_name, double duration, double ticks_per_seconds) {
		Animation * anim = new Animation{ anim_count++, anim_name, duration, ticks_per_seconds };
        anims[anim->id] = anim;
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
            LOG((ostringstream() << "ERROR::ASSIMP:: " << assimp_importer.GetErrorString()).str());
            return;
        }

        for (size_t i = 0; i < scene->mNumAnimations; i++)
        {
            auto& aianim = scene->mAnimations[i];
            string anim_name = string(aianim->mName.C_Str());
            if (aianim->mNumChannels < 1) {
                continue;
            }
            Animation* anim = CreateAnimation( anim_name, aianim->mDuration, aianim->mTicksPerSecond );
            for (size_t j = 0; j < aianim->mNumChannels; j++)
            {
                auto channel = aianim->mChannels[j];
                auto bone_name = string(channel->mNodeName.C_Str());
                auto& bone_frames = anim->bone_frames[bone_name];
                // bone at 0 index is armature name
                if (j == 0) {
                    //auto mesh_ptr = armature_name_to_mesh.find(bone_name);
                    //if (mesh_ptr == armature_name_to_mesh.end()) {
                    //    LOG((ostringstream() << "mesh with armature '" << bone_name << "' not found for animation '" << anim_name << "'").str());
                    //    break;
                    //}
                    //mesh = mesh_ptr->second;

                    if (mesh->armature->name != bone_name) {
                        LOG((ostringstream()
                            << "armature name '" << mesh->armature->name
                            << "' for mesh '" << mesh->name
                            << "' not matching animation armature name'"
                            << bone_name << "'")
                            .str());
                    };
                }
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
};