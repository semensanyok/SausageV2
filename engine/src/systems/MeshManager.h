#pragma once

#include "sausage.h"
#include "Structures.h"
#include "ThreadSafeNumberPool.h"
#include "MeshDataStruct.h"
#include "AnimationStruct.h"
#include "PhysicsStruct.h"
#include "LightStruct.h"
#include "Interfaces.h"
#include "Texture.h"
#include "AssimpHelper.h"

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
  ThreadSafeNumberPool* mesh_id_pool;
  ThreadSafeNumberPool* bone_id_pool;
  unordered_map<unsigned long, MeshDataBase*> all_meshes;
  // base_mesh_id -> instance_id -> mesh
  unordered_map<unsigned long,
    unordered_map<unsigned long, MeshDataInstance*>> instances_by_base_mesh_id;
  vector<Armature*> all_armatures;
  vector<Light*> all_lights;
 public:
   MeshManager();
   ~MeshManager();


  void DeleteMeshData(MeshDataBase* mesh);
  void DeleteMeshDataInstance(MeshDataInstance* mesh);

  void Reset();
  
  void LoadMeshes(const string& file_name,
                  vector<Light*>& out_lights,
                  vector<shared_ptr<MeshLoadData<Vertex>>>& out_mesh_load_data_animated,
                  vector<shared_ptr<MeshLoadData<VertexStatic>>>& out_mesh_load_data_static,
                  vector<MaterialTexNames>& out_tex_names_animated,
                  vector<MaterialTexNames>& out_tex_names_static,
                  bool is_load_transform = true,
                  bool is_load_aabb = true,
                  bool is_load_armature = false);

  Bone CreateBone(string bone_name, mat4& offset, mat4& trans);
  MeshData* CreateMeshData();
  MeshDataOverlay3D* CreateMeshDataFont3D(string& text, mat4& transform);
  MeshDataUI* CreateMeshDataFontUI(vec2 transform, Texture* texture = nullptr);
  MeshDataStatic* CreateMeshDataStatic();
  MeshDataOutline* CreateMeshDataOutline();

  template<typename VERTEX_TYPE>
  MeshData* CreateMeshData(MeshLoadData<VERTEX_TYPE>* load_data) {
    auto mesh = new MeshData(mesh_id_pool->ObtainNumber(), load_data);
    all_meshes[mesh->id] = mesh;
    return mesh;
  };

  MeshDataInstance* CreateInstancedMesh(MeshDataBase * base_mesh, const unsigned long instance_id);
  MeshDataInstance* CreateInstancedMesh(MeshDataBase* base_mesh, const unsigned long instance_id, mat4& transform);

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadDataFromVertices(vector<VERTEX_TYPE>& vertices,
                                      vector<unsigned int>& indices,
                                      Armature* armature = nullptr) {
    auto mld_ptr =
      new MeshLoadData<VERTEX_TYPE>{ armature, (PhysicsData*)nullptr, string(), mat4(1), vertices, indices };
    return shared_ptr<MeshLoadData<VERTEX_TYPE>>(mld_ptr);
  };

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<float>& vertices,
                                      vector<unsigned int>& indices) {
    vector<VERTEX_TYPE> positions;
    for (int i = 0; i < vertices.size(); i += 3) {
      VERTEX_TYPE vert;
      vert.Position = vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
      positions.push_back(vert);
    }
    return CreateLoadDataFromVertices(positions, indices);
  }

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<vec3>& vertices,
                                      vector<unsigned int>& indices) {
    vector<vec3> empty;
    return CreateLoadData<VERTEX_TYPE>(vertices, indices, empty);
  };

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals) {
    vector<VERTEX_TYPE> positions;
    for (int i = 0; i < vertices.size(); i++) {
      VERTEX_TYPE vert;
      vert.Position = vertices[i];
      if (!normals.empty()) {
        vert.Normal = normals[i];
      }
      positions.push_back(vert);
    }
    return CreateLoadDataFromVertices<VERTEX_TYPE>(positions, indices);
  };

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals,
                                      vector<vec2>& uvs){
    vector<VERTEX_TYPE> positions;
    for (int i = 0; i < vertices.size(); i++) {
      VERTEX_TYPE vert;
      vert.Position = vertices[i];
      if (!normals.empty()) {
        vert.Normal = normals[i];
      }
      if (!uvs.empty()) {
        vert.TexCoords = uvs[i];
      }
      positions.push_back(vert);
    }
    return CreateLoadDataFromVertices<VERTEX_TYPE>(positions, indices);
  }

  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals,
                                      vector<vec2>& uvs,
                                      vector<vec3>& tangents,
                                      vector<vec3>& bitangents) {
    vector<VERTEX_TYPE> positions;
    for (int i = 0; i < vertices.size(); i++) {
      VERTEX_TYPE vert;
      vert.Position = vertices[i];
      if (!normals.empty()) {
        vert.Normal = normals[i];
      }
      if (!uvs.empty()) {
        vert.TexCoords = uvs[i];
      }
      if (!tangents.empty()) {
        vert.Tangent = tangents[i];
      }
      if (!bitangents.empty()) {
        vert.Bitangent = bitangents[i];
      }
      positions.push_back(vert);
    }
    return CreateLoadDataFromVertices<VERTEX_TYPE>(positions, indices);
  };
  template<typename VERTEX_TYPE>
  shared_ptr<MeshLoadData<VERTEX_TYPE>> CreateLoadData(vector<vec3>& vertices,
                                      vector<unsigned int>& indices,
                                      vector<vec3>& normals,
                                      vector<vec2>& uvs,
                                      vector<uint>& uniform_id) {
    vector<VERTEX_TYPE> positions;
    for (int i = 0; i < vertices.size(); i++) {
      VERTEX_TYPE vert;
      vert.Position = vertices[i];
      if (!normals.empty()) {
        vert.Normal = normals[i];
      }
      if (!uvs.empty()) {
        vert.TexCoords = uvs[i];
      }
      if (!uniform_id.empty()) {
        vert.UniformId = uniform_id[i];
      }
      positions.push_back(vert);
    }
    return CreateLoadDataFromVertices<VERTEX_TYPE>(positions, indices);
  };

  string GetBoneName(const char* bone, Armature* armature, bool is_dae = false);

 private:
  using weight_comparator =
      decltype([](const pair<Bone*, aiVertexWeight>& lhs,
                  const pair<Bone*, aiVertexWeight>& rhs) {
        return lhs.second.mWeight > rhs.second.mWeight;
      });

  shared_ptr<MeshLoadData<Vertex>> ProcessMesh(aiMesh* mesh, const aiScene* scene,
                                       bool is_dae = false);
  shared_ptr<MeshLoadData<VertexStatic>> ProcessMeshStatic(aiMesh* mesh, const aiScene* scene,
                                       bool is_dae = false);
  aiNode* _GetBoneNode(Armature* armature, const char* bone_name,
                       const aiScene* scene, bool is_dae);
  void _LoadMeshesIndices(aiNode* child,
                           vector<unsigned int>& out_mMeshes_indices);
  void _IterChildren(aiNode* ainode);
  void _ProcessVertex(aiMesh* mesh, int i, vector<Vertex>& vertices, unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>& vertex_index_to_weights);
  void _ProcessVertexStatic(aiMesh* mesh, int i, vector<VertexStatic>& vertices);
  void _SetVertexBones(
      Vertex& vertex, int i,
      unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
          vertex_index_to_weights);
  void _SetBoneHierarchy(Armature* armature, aiNode* parent_node, Bone* parent,
                         bool is_dae = false);

  MaterialTexNames _GetTexNames(const aiMesh* mesh, const aiScene* scene,
                                bool is_obj = false);
  void _BlenderPostprocessLights(vector<Light*>& lights);
  void _ClearInstances();
};
