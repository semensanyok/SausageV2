#pragma once

#include "sausage.h"
#include "Texture.h"
#include "Structures.h"
#include "Camera.h"
#include "Animation.h"

using namespace std;
using namespace glm;

ostream& operator<<(ostream& in, const aiString& aistring) {
    in << string(aistring.C_Str());
    return in;
}

class MeshManager {
public:
    inline static atomic<unsigned long> mesh_count{ 0 };
    inline static atomic<unsigned long> bone_count{ 0 };
    inline static map<string, MeshData*> name_to_mesh;
    inline static map<string, MeshData*> armature_name_to_mesh;

    // note: 1 animation per file. (cant parse multiple anims via assimp)
    static void LoadAnimationForMesh(
        const string& file_name,
        MeshData* mesh
    ) {
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
            Animation* anim = new Animation{ anim_name, aianim->mDuration, aianim->mTicksPerSecond };
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
    static void LoadMeshes(
        const string& file_name,
        vector<Light*>& out_lights,
        vector<shared_ptr<MeshLoadData>>& out_mesh_load_data
    )
    {
        bool is_obj = file_name.ends_with(".obj");
        Assimp::Importer assimp_importer;

        const aiScene *scene = assimp_importer.ReadFile(
            file_name, aiProcess_PopulateArmatureData |
                           aiProcess_GenBoundingBoxes | aiProcess_Triangulate |
                           aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
                           aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            LOG((ostringstream() << "ERROR::ASSIMP:: " << assimp_importer.GetErrorString()).str());
            return;
        }

        for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++) {
            auto child = scene->mRootNode->mChildren[i];
            auto ai_t = child->mTransformation;
            auto transform = mat4(
                ai_t.a1, ai_t.b1, ai_t.c1, ai_t.d1,
                ai_t.a2, ai_t.b2, ai_t.c2, ai_t.d2,
                ai_t.a3, ai_t.b3, ai_t.c3, ai_t.d3,
                ai_t.a4, ai_t.b4, ai_t.c4, ai_t.d4);

            for (unsigned int j = 0; j < child->mNumMeshes; j++)
            {
                // the node object only contains indices to index the actual objects in the scene. 
                // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
                aiMesh* mesh = scene->mMeshes[child->mMeshes[j]];

                auto data_ptr = ProcessMesh(mesh, scene);
                auto data = data_ptr.get();
                data->mesh_data->transform = transform;
                data->mesh_data->max_AABB = FromAi(mesh->mAABB.mMax);
                data->mesh_data->min_AABB = FromAi(mesh->mAABB.mMin);

                data->mesh_data->name = string(mesh->mName.C_Str());
                data->tex_names = _GetTexNames(mesh, scene, is_obj);
                out_mesh_load_data.push_back(data_ptr);
                name_to_mesh[data->mesh_data->name] = data->mesh_data;
                if (data->mesh_data->armature != nullptr) {
                    armature_name_to_mesh[data->mesh_data->armature->name] = data->mesh_data;
                }
            }
        }
        
        for (unsigned int i = 0; i < scene->mNumLights; i++) {
            auto light = scene->mLights[i];
            auto node = scene->mRootNode->FindNode(light->mName);
            auto tr = node->mTransformation;
            auto res_light = FromAi(light);
            // -Z forward Y up in Blender export settings
            res_light->position = vec4(tr.a4, tr.b4, tr.c4, 0.0);
            out_lights.push_back(res_light);
        }
    }
    static Light* FromAi(aiLight* light) {
        switch (light->mType)
        {
        case aiLightSource_DIRECTIONAL:
            return new Light{ FromAi(light->mDirection),
                             FromAi(light->mPosition),
                             FromAi(light->mColorDiffuse),
                             FromAi(light->mColorSpecular),
                             LightType::Directional,
                             0,
                             0,
                             0,
                             0,
                             0 };
        case aiLightSource_POINT:
            return new Light{ FromAi(light->mDirection),
                             FromAi(light->mPosition),
                             FromAi(light->mColorDiffuse),
                             FromAi(light->mColorSpecular),
                             LightType::Point,
                             0,
                             0,
                             light->mAttenuationConstant,
                             light->mAttenuationLinear,
                             light->mAttenuationQuadratic };
        case aiLightSource_SPOT:
            return new Light{ FromAi(light->mDirection),
                             FromAi(light->mPosition),
                             FromAi(light->mColorDiffuse),
                             FromAi(light->mColorSpecular),
                             LightType::Spot,
                             cos(light->mAngleInnerCone),
                             cos(light->mAngleOuterCone),
                             light->mAttenuationConstant,
                             light->mAttenuationLinear,
                             light->mAttenuationQuadratic };
        default:
            return new Light{ FromAi(light->mDirection),
                             FromAi(light->mPosition),
                             FromAi(light->mColorDiffuse),
                             FromAi(light->mColorSpecular),
                             LightType::Directional,
                             0,
                             0,
                             0,
                             0,
                             0 };
        }
    }
    static vec4 FromAi(aiQuaternion& aivec) {
        return vec4(aivec.x, aivec.y, aivec.z, aivec.w);
    }
    static vec4 FromAi(aiVector3D& aivec) {
        return vec4(aivec.x, aivec.y, aivec.z, 0);
    }
    static vec4 FromAi(aiColor3D& aivec) {
        return vec4(aivec.r, aivec.g, aivec.b, 0);
    }
    static Bone CreateBone(const char* bone_name, mat4& offset) {
        return { bone_count++, bone_name, offset };
    }
    static shared_ptr<MeshLoadData> CreateMesh(vector<Vertex> &vertices,
                                               vector<unsigned int> &indices,
                                               Armature *armature = nullptr,
                                               bool is_new_mesh_data = true) {
      MeshData *mesh_data = nullptr;
      if (is_new_mesh_data) {
        mesh_data = new MeshData();
        mesh_data->id = mesh_count++;
        mesh_data->buffer_id = -1;
        mesh_data->instance_id = 0;
        if (armature != nullptr) {
            mesh_data->armature = armature;
        }
      }
      return make_shared<MeshLoadData>(mesh_data, vertices, indices,
                                       MaterialTexNames(), 1);
    };

    static shared_ptr<MeshLoadData> CreateMesh(vector<float> &vertices,
                                               vector<unsigned int> &indices,
                                               bool is_new_mesh_data = true) {
      vector<Vertex> positions;
      for (int i = 0; i < vertices.size(); i += 3) {
        Vertex vert;
        vert.Position = vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        positions.push_back(vert);
      }
      return CreateMesh(positions, indices, nullptr, is_new_mesh_data);
    };
    static shared_ptr<MeshLoadData> CreateMesh(vector<vec3> &vertices,
                                               vector<unsigned int> &indices,
                                               bool is_new_mesh_data = true) {
      vector<vec3> empty;
      return CreateMesh(vertices, indices, empty, is_new_mesh_data);
    };
    static shared_ptr<MeshLoadData> CreateMesh(vector<vec3> &vertices,
                                               vector<unsigned int> &indices,
                                               vector<vec3> &normals,
                                               bool is_new_mesh_data = true) {
      vector<Vertex> positions;
      for (int i = 0; i < vertices.size(); i++) {
        Vertex vert;
        vert.Position = vertices[i];
        if (!normals.empty()) {
          vert.Normal = normals[i];
        }
        positions.push_back(vert);
      }
      return CreateMesh(positions, indices, nullptr, is_new_mesh_data);
    };

private:
    static shared_ptr<MeshLoadData> ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        Bone* bones = new Bone[mesh->mNumBones];
        Armature* armature = mesh->mNumBones > 0 ? new Armature{ {}, mesh->mNumBones, bones, {} } : nullptr;

        map<unsigned int,
            set<pair<Bone *, aiVertexWeight>,
                decltype([](const pair<Bone *, aiVertexWeight> &lhs,
                            const pair<Bone *, aiVertexWeight> &rhs) {
                  return lhs.second.mWeight > rhs.second.mWeight;
                })>>
            vertex_index_to_weights;


        for (size_t i = 0; i < mesh->mNumBones; i++)
        {
            auto aibone = mesh->mBones[i];
            if (i == 0) {
                armature->name = aibone->mArmature->mName.C_Str();
            }
            auto& ai_t = aibone->mOffsetMatrix;
            auto offset = mat4(
                ai_t.a1, ai_t.b1, ai_t.c1, ai_t.d1,
                ai_t.a2, ai_t.b2, ai_t.c2, ai_t.d2,
                ai_t.a3, ai_t.b3, ai_t.c3, ai_t.d3,
                ai_t.a4, ai_t.b4, ai_t.c4, ai_t.d4);
            bones[i] = CreateBone(aibone->mName.C_Str(), offset);
            armature->name_to_bone[bones[i].name] = &bones[i];
            for (int j = 0; j < aibone->mNumWeights; j++) {
                auto& weight = aibone->mWeights[j];
                vertex_index_to_weights[weight.mVertexId].insert({ &bones[i], weight });
            }
        }

        for (size_t i = 0; i < mesh->mNumBones; i++)
        {
            auto aibone = mesh->mBones[i];
            if (aibone->mNode->mNumChildren > 0) {
                SetBoneHierarchy(armature, aibone->mNode, armature->name_to_bone[aibone->mName.C_Str()]);
            }
        }
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };
            if (mesh->HasNormals())
            {
                vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z };
            }
            vertex.TexCoords = mesh->mTextureCoords[0] ? vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : vec2(0, 0);
            vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z };
            vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z };
            // init animation data
            vertex.BoneIds = ivec4(-1);
            vertex.BoneWeights = vec4(0.0);
            if (!vertex_index_to_weights.empty()) {
                auto& weights_v = vertex_index_to_weights[i];
                auto weight = weights_v.begin();
                if (weight != weights_v.end()) {
                    vertex.BoneIds.x = (*weight).first->id;
                    vertex.BoneWeights.x = (*weight).second.mWeight;
                    weight++;
                    if (weight != weights_v.end()) {
                        vertex.BoneIds.y = (*weight).first->id;
                        vertex.BoneWeights.y = (*weight).second.mWeight;
                        weight++;
                        if (weight != weights_v.end()) {
                            vertex.BoneIds.z = (*weight).first->id;
                            vertex.BoneWeights.z = (*weight).second.mWeight;
                            weight++;
                            if (weight != weights_v.end()) {
                                vertex.BoneIds.w = (*weight).first->id;
                                vertex.BoneWeights.w = (*weight).second.mWeight;
                                weight++;
                            }
                        }
                    }
                }
            }
            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        return CreateMesh(vertices, indices, armature);
    }
    static void SetBoneHierarchy(Armature* armature, aiNode* parent_node, Bone* parent) {
        for (size_t j = 0; j < parent_node->mNumChildren; j++)
        {
            if (j == 0) {
                parent->children.reserve(parent_node->mNumChildren);
            }
            auto child_node = parent_node->mChildren[j];
            auto child_bone = armature->name_to_bone.find(child_node->mName.C_Str());
            if (child_bone != armature->name_to_bone.end()) {
                parent->children.push_back((*child_bone).second);
                SetBoneHierarchy(armature, child_node, (*child_bone).second);
            }
        }
    }

    static MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene, bool is_obj = false) {
        string diffuse_name;
        string normal_name;
        string specular_name;
        string height_name;
        string metal_name;
        string ao_name;
        string opacity_name;

        aiString Path;
        auto mat = scene->mMaterials[mesh->mMaterialIndex];
        if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            diffuse_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            normal_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            specular_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            // Blender obj export saves .mtl with normal tex as bump tex.
            if (is_obj) {
                normal_name = filesystem::path(Path.C_Str()).filename().string();
            }
            else {
                height_name = filesystem::path(Path.C_Str()).filename().string();
            }
        }
        if (mat->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            metal_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            ao_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_OPACITY, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            opacity_name = filesystem::path(Path.C_Str()).filename().string();
        }
        return MaterialTexNames(diffuse_name, normal_name, specular_name, height_name, metal_name, ao_name, opacity_name);
    }
    MeshManager() = delete;
    ~MeshManager() = delete;
};
