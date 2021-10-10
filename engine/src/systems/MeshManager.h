#pragma once

#include "Structures.h"
#include "Texture.h"
#include "sausage.h"
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
  MeshManager(){};
  ~MeshManager(){};
  atomic<unsigned long> mesh_count{0};
  atomic<unsigned long> bone_count{0};

  vector<MeshDataBase*> meshes;
  vector<Armature*> armatures;
  vector<Light*> all_lights;

  void Reset();
  
  void LoadMeshes(const string& file_name,
                  vector<Light*>& out_lights,
                  vector<shared_ptr<MeshLoadData>>& out_mesh_load_data,
                  bool is_load_transform = true,
                  bool is_load_aabb = true,
                  bool is_load_armature = false);

  void __LoadMeshesIndices(aiNode* child,
                           vector<unsigned int>& out_mMeshes_indices);

  Bone CreateBone(string bone_name, mat4& offset, mat4& trans);
  MeshData* CreateMeshData() {
    auto mesh = new MeshData();
    mesh->id = mesh_count++;
    mesh->buffer_id = -1;
    mesh->instance_id = 0;
    meshes.push_back(mesh);
    return mesh;
  };
  MeshDataFont3D* CreateMeshDataFont() {
    auto mesh = new MeshDataFont3D();
    mesh->id = mesh_count++;
    mesh->buffer_id = -1;
    mesh->instance_id = 0;
    meshes.push_back(mesh);
    return mesh;
  };
  MeshDataFontUI* CreateMeshDataFontUI(string text, vec2 transform) {
    auto mesh = new MeshDataFontUI(text, transform);
    mesh->id = mesh_count++;
    mesh->buffer_id = -1;
    mesh->instance_id = 0;
    mesh->transform = transform;
    meshes.push_back(mesh);
    return mesh;
  };
  shared_ptr<MeshLoadData> CreateMesh(vector<Vertex>& vertices,
                                      vector<unsigned int>& indices,
                                      Armature* armature = nullptr);
  shared_ptr<MeshLoadData> CreateMesh(vector<float>& vertices,
                                      vector<unsigned int>& indices);
  shared_ptr<MeshLoadData> CreateMesh(vector<vec3>& vertices,
                                      vector<unsigned int>& indices);
  shared_ptr<MeshLoadData> CreateMesh(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals);
  shared_ptr<MeshLoadData> CreateMesh(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals,
                                      vector<vec2>& uvs);
  string GetBoneName(const char* bone, Armature* armature, bool is_dae = false);

 private:
  using weight_comparator =
      decltype([](const pair<Bone*, aiVertexWeight>& lhs,
                  const pair<Bone*, aiVertexWeight>& rhs) {
        return lhs.second.mWeight > rhs.second.mWeight;
      });
  
  shared_ptr<MeshLoadData> ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                       bool is_load_armature = false,
                                       bool is_dae = false);
  aiNode* _GetBoneNode(Armature* armature, const char* bone_name,
                       const aiScene* scene, bool is_dae);
  void _IterChildren(aiNode* ainode);
  void _ProcessVertex(
      aiMesh* mesh, int i, vector<Vertex>& vertices,
      map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
          vertex_index_to_weights,
      bool is_load_armature = false);
  void _SetVertexBones(
      Vertex& vertex, int i,
      map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
          vertex_index_to_weights);
  void _SetBoneHierarchy(Armature* armature, aiNode* parent_node, Bone* parent,
                         bool is_dae = false);

  MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene,
                                bool is_obj = false);
  void _BlenderPostprocessLights(vector<Light*>& lights);
};
