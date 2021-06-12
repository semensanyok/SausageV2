#pragma once

#include "sausage.h"
#include "Texture.h"
#include "Structures.h"
#include "systems/Camera.h"

using namespace std;
using namespace glm;

static atomic<unsigned int> mesh_count{ 0 };

class MeshManager {
public:
    // Structure of arrays style for multidraw.
    static void LoadMeshes(
        vector<vector<Vertex>>& vertices,
        vector<vector<unsigned int>>& indices,
        const string& file_name,
        vector<MeshData>& out_mesh_data,
        vector<Light>& out_lights,
        map<unsigned int, MaterialTexNames>& out_mesh_id_to_tex
    )
    {
        Assimp::Importer assimp_importer;

        const aiScene* scene = assimp_importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
        {
            cout << "ERROR::ASSIMP:: " << assimp_importer.GetErrorString() << endl;
            throw runtime_error(string("ERROR::ASSIMP:: ") += assimp_importer.GetErrorString());
        }

        for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++) {
            auto child = scene->mRootNode->mChildren[i];
            auto ai_t = child->mTransformation;
            auto model = mat4(
                ai_t.a1, ai_t.b1, ai_t.c1, ai_t.d1,
                ai_t.a2, ai_t.b2, ai_t.c2, ai_t.d2,
                ai_t.a3, ai_t.b3, ai_t.c3, ai_t.d3,
                ai_t.a4, ai_t.b4, ai_t.c4, ai_t.d4);
            for (unsigned int j = 0; j < child->mNumMeshes; j++)
            {
                // the node object only contains indices to index the actual objects in the scene. 
                // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
                aiMesh* mesh = scene->mMeshes[child->mMeshes[j]];
                MaterialTexNames tex_names = _GetTexNames(mesh, scene);

                auto data = ProcessMesh(mesh, scene);
                vertices.push_back(data.vertices);
                indices.push_back(data.indices);
                MeshData mesh_data = MeshData(data.draw_id, model);
                out_mesh_data.push_back(mesh_data);
                out_mesh_id_to_tex[mesh_data.id] = tex_names;
            }
        }
        for (unsigned int i = 0; i < scene->mNumLights; i++) {
            auto light = scene->mLights[i];
            out_lights.push_back(FromAi(scene->mLights[i]));
        }
    }
    static Light FromAi(aiLight* light) {
        switch (light->mType)
        {
        case aiLightSource_DIRECTIONAL:
            return Light{FromAi(light->mDirection),FromAi(light->mPosition),0,FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular), LightType::Directional };
        case aiLightSource_POINT:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),0,FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular), LightType::Point };
        case aiLightSource_SPOT:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),light->mAngleOuterCone,FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular), LightType::Spot };
        default:
            return Light{ FromAi(light->mDirection),FromAi(light->mPosition),0,FromAi(light->mColorDiffuse), FromAi(light->mColorSpecular), LightType::Directional };
        }
    }
    static vec3 FromAi(aiVector3D& aivec) {
        return vec3(aivec.x, aivec.y, aivec.z);
    }
    static vec3 FromAi(aiColor3D& aivec) {
        return vec3(aivec.r, aivec.g, aivec.b);
    }
    static MeshLoadData CreateMesh(vector<Vertex>& vertices, vector<unsigned int>& indices) {
        return MeshLoadData{ vertices, indices, mesh_count++ };
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
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vertex.Bitangent = vec3(
                    mesh->mBitangents[i].x,
                    mesh->mBitangents[i].y,
                    mesh->mBitangents[i].z);
            }
            else
                vertex.TexCoords = vec2(0.0f, 0.0f);
            vertices.push_back(vertex);
        }
        // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }
        // return a mesh object created from the extracted mesh data
        return CreateMesh(vertices, indices);
    }

    static MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene) {
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
            cout << string(Path.C_Str()) << endl;
            diffuse_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << string(Path.C_Str()) << endl;
            normal_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << "SPEC" << string(Path.C_Str()) << endl;
            specular_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << string(Path.C_Str()) << endl;
            height_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << "METAL" << string(Path.C_Str()) << endl;
            metal_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << "AMBIENT" << string(Path.C_Str()) << endl;
            ao_name = string(Path.C_Str());
        }
        if (mat->GetTexture(aiTextureType_OPACITY, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
            cout << "OPACITY" << string(Path.C_Str()) << endl;
            opacity_name = string(Path.C_Str());
        }
        return MaterialTexNames(diffuse_name, normal_name, specular_name, height_name, metal_name, ao_name, opacity_name);
    }
    MeshManager() = delete;
    ~MeshManager() = delete;
};