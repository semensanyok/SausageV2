#pragma once

#include "Texture.h"
#include "Structures.h"
#include <vector>
#include <atomic>

using namespace std;
using namespace glm;

static atomic<unsigned int> mesh_count{ 0 };

MeshLoadData CreateMesh(vector<Vertex>& vertices, vector<unsigned int>& indices) {
    return MeshLoadData{ vertices, indices, ++mesh_count };
};

MeshLoadData CreateMesh(vector<float>& vertices, vector<unsigned int>& indices) {
    vector<Vertex> positions;
    for (int i = 0; i < vertices.size(); i += 3) {
        Vertex vert;
        vert.Position = vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
        positions.push_back(vert);
    }
    return CreateMesh(positions, indices);
};

MeshLoadData ProcessMesh(aiMesh* mesh, const aiScene* scene)
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
            //vector.x = mesh->mBitangents[i].x;
            //vector.y = mesh->mBitangents[i].y;
            //vector.z = mesh->mBitangents[i].z;
            //vertex.Bitangent = vector;
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

// Structure of arrays style for multidraw.
void LoadMeshes(vector<vector<Vertex>>& vertices, vector<vector<unsigned int>>& indices, vector<unsigned int>& draw_ids, const string& file_name)
{
    Assimp::Importer assimp_importer;

    const aiScene* scene = assimp_importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        cout << "ERROR::ASSIMP:: " << assimp_importer.GetErrorString() << endl;
        throw runtime_error(string("ERROR::ASSIMP:: ") += assimp_importer.GetErrorString());
    }
    // process each mesh located at the current node
    for (unsigned int i = 0; i < scene->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[i];
        MeshLoadData data = ProcessMesh(mesh, scene);
        vertices.push_back(data.vertices);
        indices.push_back(data.indices);
        draw_ids.push_back(data.draw_id);
    }
}

