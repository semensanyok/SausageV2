#pragma once

#include "Structures.h"
#include "MeshDataStruct.h"
#include "AnimationStruct.h"
#include "PhysicsStruct.h"
#include "LightStruct.h"
#include "Interfaces.h"
#include "Texture.h"
#include "sausage.h"
#include "AssimpHelper.h"
#include "ThreadSafeNumberPool.h"

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

class MeshManager : public SausageSystem {
 public:
   MeshManager();
   ~MeshManager();
  ThreadSafeNumberPool* mesh_id_pool;
  ThreadSafeNumberPool* bone_id_pool;

  vector<MeshDataBase*> all_meshes;
  vector<Armature*> all_armatures;
  vector<Light*> all_lights;

  void DeleteMeshData(MeshDataBase* mesh);

  void Reset();
  
  void LoadMeshes(const string& file_name,
                  vector<Light*>& out_lights,
                  vector<shared_ptr<MeshLoadData>>& out_mesh_load_data,
                  vector<MaterialTexNames>& out_tex_names,
                  bool is_load_transform = true,
                  bool is_load_aabb = true,
                  bool is_load_armature = false);

  Bone CreateBone(string bone_name, mat4& offset, mat4& trans);
  MeshData* CreateMeshData();
  MeshData* CreateInstancedMesh(MeshData* base_mesh);;
  MeshData* CreateMeshData(MeshLoadData* load_data);
  MeshDataOverlay3D* CreateMeshDataFont3D(string& text, mat4& transform);
  MeshDataUI* CreateMeshDataFontUI(vec2 transform, Texture* texture = nullptr);
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
  shared_ptr<MeshLoadData> CreateMesh(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals,
                                      vector<vec2>& uvs,
                                      vector<vec3>& tangents);
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
  void _LoadMeshesIndices(aiNode* child,
                           vector<unsigned int>& out_mMeshes_indices);
  void _IterChildren(aiNode* ainode);
  void _ProcessVertex(
      aiMesh* mesh, int i, vector<Vertex>& vertices,
      unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
          vertex_index_to_weights,
      bool is_load_armature = false);
  void _SetVertexBones(
      Vertex& vertex, int i,
      unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
          vertex_index_to_weights);
  void _SetBoneHierarchy(Armature* armature, aiNode* parent_node, Bone* parent,
                         bool is_dae = false);

  MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene,
                                bool is_obj = false);
  void _BlenderPostprocessLights(vector<Light*>& lights);
};
