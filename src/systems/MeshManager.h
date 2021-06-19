#pragma once

#include "sausage.h"
#include "Texture.h"
#include "Structures.h"
#include "systems/Camera.h"

using namespace std;
using namespace glm;

class MeshManager {
public:
    inline static atomic<unsigned int> mesh_count{ 0 };
    // Structure of arrays style for multidraw.
    static void LoadMeshes(
        const string& file_name,
        vector<Light>& out_lights,
        vector<MeshLoadData>& out_mesh_load_data
    )
    {
        bool is_obj = file_name.ends_with(".obj");
        Assimp::Importer assimp_importer;

        const aiScene* scene = assimp_importer.ReadFile(file_name, aiProcess_GenBoundingBoxes | aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

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

                auto data = ProcessMesh(mesh, scene);
                data.mesh_data.transform = transform;
                data.mesh_data.max_AABB = mat3(transform) * FromAi(mesh->mAABB.mMax);
                data.mesh_data.min_AABB = mat3(transform) * FromAi(mesh->mAABB.mMin);
                data.mesh_data.name = string(mesh->mName.C_Str());
                data.tex_names = _GetTexNames(mesh, scene, is_obj);
                out_mesh_load_data.push_back(data);
            }
        }
        for (unsigned int i = 0; i < scene->mNumLights; i++) {
            auto light = scene->mLights[i];
            auto node = scene->mRootNode->FindNode(light->mName);
            auto tr = node->mTransformation;
            auto res_light = FromAi(light);
            // -Z forward Y up in Blender export settings
            res_light.position = vec4(tr.a4, tr.b4, tr.c4, 0.0);
            out_lights.push_back(res_light);
        }
    }
    static Light FromAi(aiLight* light) {
        switch (light->mType)
        {
        case aiLightSource_DIRECTIONAL:
            return Light{FromAi(light->mDirection),FromAi(light->mPosition),FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular),LightType::Directional,0,0,0,0,0};
        case aiLightSource_POINT:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular),LightType::Point,0,0,light->mAttenuationConstant,light->mAttenuationLinear,light->mAttenuationQuadratic };
        case aiLightSource_SPOT:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular),LightType::Spot,cos(light->mAngleInnerCone),cos(light->mAngleOuterCone),light->mAttenuationConstant,light->mAttenuationLinear,light->mAttenuationQuadratic };
        default:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular),LightType::Directional,0,0,0,0,0 };
        }
    }
    static vec4 FromAi(aiVector3D& aivec) {
        return vec4(aivec.x, aivec.y, aivec.z, 0);
    }
    static vec4 FromAi(aiColor3D& aivec) {
        return vec4(aivec.r, aivec.g, aivec.b, 0);
    }
    static MeshLoadData CreateMesh(vector<Vertex>& vertices, vector<unsigned int>& indices) {
        MeshData mesh_data;
        mesh_data.id = mesh_count++;
        mesh_data.instance_id = 0;
        return MeshLoadData{ mesh_data, vertices, indices, MaterialTexNames(), 1 };
    };

    static MeshLoadData CreateMesh(vector<float>& vertices, vector<unsigned int>& indices) {
        vector<Vertex> positions;
        for (int i = 0; i < vertices.size(); i += 3) {
            Vertex vert;
            vert.Position = vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
            positions.push_back(vert);
        }
        return CreateMesh(positions, indices);
    };
private:

    static MeshLoadData ProcessMesh(aiMesh* mesh, const aiScene* scene)
    {
        // data to fill
        vector<Vertex> vertices;
        vector<unsigned int> indices;

        // walk through each of the mesh's vertices
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder vec3 first.
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                vertex.Bitangent = vec3(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z);
            }
            else
                vertex.TexCoords = vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        return CreateMesh(vertices, indices);
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
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            diffuse_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            normal_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            specular_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            // Blender obj export saves .mtl with normal tex as bump tex.
            if (is_obj) {
                normal_name = filesystem::path(Path.C_Str()).filename().string();
            }
            else {
                height_name = filesystem::path(Path.C_Str()).filename().string();
            }
        }
        if (mat->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            metal_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            ao_name = filesystem::path(Path.C_Str()).filename().string();
        }
        if (mat->GetTexture(aiTextureType_OPACITY, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << filesystem::path(Path.C_Str()).filename() << endl;
            opacity_name = filesystem::path(Path.C_Str()).filename().string();
        }
        return MaterialTexNames(diffuse_name, normal_name, specular_name, height_name, metal_name, ao_name, opacity_name);
    }
    MeshManager() = delete;
    ~MeshManager() = delete;
};