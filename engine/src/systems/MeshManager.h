#pragma once

#include "sausage.h"
#include "Texture.h"
#include "Structures.h"
#include "utils/AssimpHelper.h"

#define AI_MAX_BONE_WEIGHTS 4
using namespace std;
using namespace glm;

inline ostream& operator<<(ostream& in, const aiString& aistring) {
    in << string(aistring.C_Str());
    return in;
}

mat4 FromAi(aiMatrix4x4& ai_t);
quat FromAi(aiQuaternion& aivec);

vec4 FromAi(aiVector3D& aivec);
vec4 FromAi(aiColor3D& aivec);

Light* FromAi(aiLight* light);

class MeshManager {
public:
    MeshManager() {};
    ~MeshManager() {};
    atomic<unsigned long> mesh_count{ 0 };
    atomic<unsigned long> bone_count{ 0 };

    map<string, MeshData*> name_to_mesh;
    map<string, MeshData*> armature_name_to_mesh;
    vector<Light*> all_lights;

    void Reset();

    void LoadMeshes(
        const string& file_name,
        vector<Light*>& out_lights,
        vector<shared_ptr<MeshLoadData>>& out_mesh_load_data,
        bool is_load_armature = false
    );

    void __LoadMeshesIndices(aiNode* child, vector<unsigned int>& out_mMeshes_indices);
    
    Bone CreateBone(string bone_name, mat4& offset, mat4& trans);
    shared_ptr<MeshLoadData> CreateMesh(vector<Vertex> &vertices,
                                               vector<unsigned int> &indices,
                                               Armature *armature = nullptr,
        bool is_new_mesh_data = true);

    shared_ptr<MeshLoadData> CreateMesh(vector<float> &vertices,
                                               vector<unsigned int> &indices,
        bool is_new_mesh_data = true);
    shared_ptr<MeshLoadData> CreateMesh(vector<vec3> &vertices,
                                               vector<unsigned int> &indices,
        bool is_new_mesh_data = true);
    shared_ptr<MeshLoadData> CreateMesh(vector<vec3> &vertices,
                                               vector<unsigned int> &indices,
                                               vector<vec3> &normals,
        bool is_new_mesh_data = true);

    string GetBoneName(const char* bone, Armature* armature, bool is_dae = false);
private:
    using weight_comparator = decltype([](const pair<Bone*, aiVertexWeight>& lhs, const pair<Bone*, aiVertexWeight>& rhs) {
        return lhs.second.mWeight > rhs.second.mWeight;
        });
    shared_ptr<MeshLoadData> ProcessMesh(aiMesh* mesh,
        const aiScene* scene,
        bool is_load_armature = false,
        bool is_dae = false);
    aiNode* _GetBoneNode(Armature* armature, const char* bone_name, const aiScene* scene, bool is_dae);
    void _IterChildren(aiNode* ainode);
    void _ProcessVertex(aiMesh* mesh,
        int i,
        vector<Vertex>& vertices,
        map<unsigned int, set < pair<Bone*, aiVertexWeight>, weight_comparator>>& vertex_index_to_weights,
        bool is_load_armature = false);
    void _SetVertexBones(Vertex& vertex,
        int i,
        map<unsigned int, set < pair<Bone*, aiVertexWeight>, weight_comparator>>& vertex_index_to_weights);
    void _SetBoneHierarchy(Armature* armature, aiNode* parent_node, Bone* parent, bool is_dae = false);

    MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene, bool is_obj = false);
    void _BlenderPostprocessLights(vector<Light*>& lights);
};
