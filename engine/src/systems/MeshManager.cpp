#include "MeshManager.h"
#include "Macros.h"

mat4 FromAi(aiMatrix4x4& ai_t) {
  return mat4(ai_t.a1, ai_t.b1, ai_t.c1, ai_t.d1, ai_t.a2, ai_t.b2, ai_t.c2,
              ai_t.d2, ai_t.a3, ai_t.b3, ai_t.c3, ai_t.d3, ai_t.a4, ai_t.b4,
              ai_t.c4, ai_t.d4);
}

quat FromAi(aiQuaternion& aivec) {
  return quat(aivec.w, aivec.x, aivec.y, aivec.z);
}

vec4 FromAi(aiVector3D& aivec) { return vec4(aivec.x, aivec.y, aivec.z, 0); }

vec4 FromAi(aiColor3D& aivec) { return vec4(aivec.r, aivec.g, aivec.b, 0); }

Light* FromAi(aiLight* light) {
  switch (light->mType) {
  case aiLightSource_DIRECTIONAL:
    return new Light{FromAi(light->mDirection),
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
    return new Light{FromAi(light->mDirection),
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
    return new Light{FromAi(light->mDirection),
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
    return new Light{FromAi(light->mDirection),
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

void MeshManager::Reset() {
  for (auto mesh : all_meshes) {
    if (mesh != NULL && mesh != nullptr)
      DeleteMeshData(mesh);
  }
  for (auto armature : all_armatures) {
    if (armature != NULL && armature != nullptr)
      delete armature;
  }
  for (auto light : all_lights) {
    if (light != NULL && light != nullptr)
      delete light;
  }
  all_meshes.clear();
  all_armatures.clear();
  all_lights.clear();
}

void MeshManager::LoadMeshes(
    const string& file_name,
    vector<Light*>& out_lights,
    vector<shared_ptr<MeshLoadData>>& out_mesh_load_data,
    vector<MaterialTexNames>& out_tex_names,
    bool is_load_armature,
    bool is_load_transform,
    bool is_load_aabb) {
  bool is_obj = file_name.ends_with(".obj");
  bool is_dae = file_name.ends_with(".dae");
  bool is_gltf = file_name.ends_with(".glb") || file_name.ends_with(".gltf");

  ConfiguredAssmipImporter

    auto flags = aiProcess_GenBoundingBoxes | aiProcess_Triangulate |
    aiProcess_GenSmoothNormals | aiProcess_FlipUVs |
    aiProcess_CalcTangentSpace;
  flags = is_load_armature ? flags | aiProcess_PopulateArmatureData : flags;
  const aiScene* scene = assimp_importer.ReadFile(file_name, flags);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode)  // if is Not Zero
  {
    LOG((ostringstream() << "ERROR::ASSIMP:: "
      << assimp_importer.GetErrorString())
            .str());
    return;
  }
  for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++) {
    auto child = scene->mRootNode->mChildren[i];
    auto transform = FromAi(child->mTransformation);
    vector<unsigned int> mMeshes_indices;
    _LoadMeshesIndices(child, mMeshes_indices);
    // if armature - no meshes, but child has meshes. (.dae and .gltf)
    if (mMeshes_indices.empty()) {
      for (unsigned int j = 0; j < child->mNumChildren; j++) {
        _LoadMeshesIndices(child->mChildren[j], mMeshes_indices);
      }
    }
    for (unsigned int j = 0; j < mMeshes_indices.size(); j++) {
      // the node object only contains indices to index the actual objects in
      // the scene. the scene contains all the data, node is just to keep stuff
      // organized (like relations between nodes).
      aiMesh* mesh = scene->mMeshes[mMeshes_indices[j]];

      auto data_ptr = ProcessMesh(mesh, scene, is_load_armature, is_dae);
      auto data = data_ptr.get();
      if (is_load_transform) {
        data->transform = transform;
      }
      if (is_load_aabb) {
        data->physics_data =
          new PhysicsData(FromAi(mesh->mAABB.mMin), FromAi(mesh->mAABB.mMax));
      }
      data->name = string(mesh->mName.C_Str());
      out_mesh_load_data.push_back(data_ptr);
      out_tex_names.push_back(_GetTexNames(mesh, scene, is_obj));
    }
  }

  for (unsigned int i = 0; i < scene->mNumLights; i++) {
    auto light = scene->mLights[i];
    auto node = scene->mRootNode->FindNode(light->mName);
    auto tr = node->mTransformation;
    auto res_light = FromAi(light);
    // -Z forward Y up in Blender export settings
    res_light->position = vec4(tr.a4, tr.b4, tr.c4, 0.0);
    all_lights.push_back(res_light);
    out_lights.push_back(res_light);
  }
  _BlenderPostprocessLights(out_lights);
}

void MeshManager::_LoadMeshesIndices(
    aiNode* child, vector<unsigned int>& out_mMeshes_indices) {
  if (child->mNumMeshes > 0) {
    for (unsigned int j = 0; j < child->mNumMeshes; j++) {
      out_mMeshes_indices.push_back(child->mMeshes[j]);
    }
  }
}

Bone MeshManager::CreateBone(string bone_name, mat4& offset, mat4& trans) {
  return { bone_id_pool->ObtainNumber(), bone_name, offset, trans, nullptr, {} };
}

MeshData* MeshManager::CreateMeshData() {
  auto mesh = new MeshData();
  mesh->id = mesh_id_pool->ObtainNumber();
  mesh->buffer_id = -1;
  mesh->instance_id = 0;
  all_meshes.push_back(mesh);
  return mesh;
}

MeshData* MeshManager::CreateInstancedMesh(MeshData* base_mesh) {
  auto* mesh = new MeshData();
  mesh->id = mesh_id_pool->ObtainNumber();
  mesh->buffer_id = base_mesh->buffer_id;
  mesh->base_mesh = base_mesh;
  // caller must set instance_id
  // via DrawCallManager.AddInstanceGetInstanceId
  mesh->instance_id = 0;
  all_meshes.push_back(mesh);
  return mesh;
}

MeshData* MeshManager::CreateMeshData(MeshLoadData* load_data) {
  auto mesh = new MeshData(load_data);
  mesh->id = mesh_id_pool->ObtainNumber();
  mesh->buffer_id = -1;
  mesh->instance_id = 0;
  all_meshes.push_back(mesh);
  return mesh;
}

MeshManager::MeshManager() {
  mesh_id_pool = new ThreadSafeNumberPool(MAX_BASE_AND_INSTANCED_MESHES);
  bone_id_pool = new ThreadSafeNumberPool(MAX_BONES);
}

MeshManager::~MeshManager() {
  delete mesh_id_pool;
  delete bone_id_pool;
}

void MeshManager::DeleteMeshData(MeshDataBase* mesh) {
  mesh_id_pool->ReleaseNumber(mesh->id);
  delete mesh;
}

MeshDataOverlay3D* MeshManager::CreateMeshDataFont3D(string& text, mat4& transform) {
  auto mesh = new MeshDataOverlay3D(text, transform);
  mesh->id = mesh_id_pool->ObtainNumber();
  mesh->buffer_id = -1;
  mesh->instance_id = 0;
  all_meshes.push_back(mesh);
  return mesh;
}

MeshDataUI* MeshManager::CreateMeshDataFontUI(vec2 transform, Texture* texture) {
  auto mesh = new MeshDataUI(transform, texture);
  mesh->id = mesh_id_pool->ObtainNumber();
  mesh->buffer_id = -1;
  mesh->instance_id = 0;
  mesh->transform = transform;
  all_meshes.push_back(mesh);
  return mesh;
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(vector<Vertex>& vertices,
                                                 vector<unsigned int>& indices,
                                                 Armature* armature) {
  return make_shared<MeshLoadData>(vertices, indices, nullptr,
                                   armature, nullptr, string(), mat4(1), 1);
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(vector<vec3>& vertices,
                                                 vector<unsigned int>& indices) {
  vector<vec3> empty;
  return CreateMesh(vertices, indices, empty);
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(vector<vec3>& vertices,
                                                 vector<unsigned int>& indices,
                                                 vector<vec3>& normals) {
  vector<Vertex> positions;
  for (int i = 0; i < vertices.size(); i++) {
    Vertex vert;
    vert.Position = vertices[i];
    if (!normals.empty()) {
      vert.Normal = normals[i];
    }
    positions.push_back(vert);
  }
  return CreateMesh(positions, indices, nullptr);
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(vector<vec3>& vertices,
                                                 vector<unsigned int>& indices,
                                                 vector<vec3>& normals,
                                                 vector<vec2>& uvs) {
  vector<Vertex> positions;
  for (int i = 0; i < vertices.size(); i++) {
    Vertex vert;
    vert.Position = vertices[i];
    if (!normals.empty()) {
      vert.Normal = normals[i];
    }
    if (!uvs.empty()) {
      vert.TexCoords = uvs[i];
    }
    positions.push_back(vert);
  }
  return CreateMesh(positions, indices, nullptr);
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(
    vector<vec3>& vertices, vector<unsigned int>& indices,
    vector<vec3>& normals, vector<vec2>& uvs, vector<vec3>& tangents) {
  vector<Vertex> positions;
  for (int i = 0; i < vertices.size(); i++) {
    Vertex vert;
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
    positions.push_back(vert);
  }
  return CreateMesh(positions, indices, nullptr);
}

string MeshManager::GetBoneName(const char* bone,
                                Armature* armature,
                                bool is_dae) {
  string name = string(bone);
  if (is_dae) {
    string arm_name = armature->name;
    size_t pos = name.find(arm_name.append("_"));
    if (pos != std::string::npos) {
      name.replace(pos, arm_name.size(), "");
    }
  }
  return name;
}

shared_ptr<MeshLoadData> MeshManager::ProcessMesh(aiMesh* mesh,
                                                  const aiScene* scene,
                                                  bool is_load_armature,
                                                  bool is_dae) {
  // data to fill
  vector<Vertex> vertices;
  vector<unsigned int> indices;
  Bone* bones = is_load_armature ? new Bone[mesh->mNumBones] : nullptr;
  Armature* armature =
    mesh->mNumBones > 0
    ? new Armature{ {}, is_load_armature ? mesh->mNumBones : 0, bones }
  : nullptr;

  unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>
    vertex_index_to_weights;
  if (is_load_armature) {
    for (size_t i = 0; i < mesh->mNumBones; i++) {
      auto aibone = mesh->mBones[i];
      auto offset = FromAi(aibone->mOffsetMatrix);
      if (i == 0) {
        armature->name = aibone->mArmature->mName.C_Str();
        if (is_dae) {
          armature->transform =
            FromAi(aibone->mArmature->mTransformation.Inverse());
        }
        else {
          armature->transform = FromAi(aibone->mArmature->mTransformation);
        }
      }
      auto bone_name = GetBoneName(aibone->mName.C_Str(), armature, is_dae);
      auto node = _GetBoneNode(armature, aibone->mName.C_Str(), scene, is_dae);
      auto trans = FromAi(node->mTransformation);
      bones[i] = CreateBone(bone_name.c_str(), offset, trans);
      armature->name_to_bone[bones[i].name] = &bones[i];
      armature->id_to_bone[bones[i].id] = &bones[i];
      for (int j = 0; j < aibone->mNumWeights; j++) {
        auto& weight = aibone->mWeights[j];
        vertex_index_to_weights[weight.mVertexId].insert({ &bones[i], weight });
      }
    }
    aiNode* armatureNode;
    for (size_t i = 0; i < mesh->mNumBones; i++) {
      auto aibone = mesh->mBones[i];
      auto node = _GetBoneNode(armature, aibone->mName.C_Str(), scene, is_dae);
      if (i == 0 && is_dae) {
        armatureNode = node;
      }
      if (node == NULL) {
        LOG((ostringstream()
          << "Bone " << string(aibone->mName.C_Str()) << " for armature "
          << armature->name << " aiNode not found in scene")
                .str());
        continue;
      }
      if (node->mNumChildren > 0) {
        auto mBoneNameFixed =
          GetBoneName(aibone->mName.C_Str(), armature, is_dae);
        auto bone = armature->name_to_bone.find(mBoneNameFixed);
        if (bone == armature->name_to_bone.end()) {
          LOG((ostringstream() << "Bone " << mBoneNameFixed << "for armature"
            << armature->name << "not found")
                  .str());
          continue;
        }
        _SetBoneHierarchy(armature, node, bone->second, is_dae);
      }
    }
  }
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    _ProcessVertex(mesh, i, vertices, vertex_index_to_weights,
                   is_load_armature);
  }
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
  }
  return CreateMesh(vertices, indices, armature);
}

aiNode* MeshManager::_GetBoneNode(Armature* armature, const char* bone_name,
                                  const aiScene* scene, bool is_dae) {
  auto node = scene->mRootNode->FindNode(bone_name);
  if (node == NULL && is_dae) {
    auto full_name = armature->name;
    full_name.append("_").append(bone_name);
    node = scene->mRootNode->FindNode(full_name.c_str());
  }
  return node;
}

void MeshManager::_IterChildren(aiNode* ainode) {
  for (size_t i = 0; i < ainode->mNumChildren; i++) {
    auto child = ainode->mChildren[i];
    _IterChildren(child);
  }
}

void MeshManager::_ProcessVertex(
    aiMesh* mesh, int i, vector<Vertex>& vertices,
    unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
        vertex_index_to_weights,
    bool is_load_armature) {
  Vertex vertex;
  vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y,
                     mesh->mVertices[i].z };
  if (mesh->HasNormals()) {
    vertex.Normal = { mesh->mNormals[i].x, mesh->mNormals[i].y,
                     mesh->mNormals[i].z };
  }
  vertex.TexCoords =
    mesh->mTextureCoords[0]
    ? vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y)
    : vec2(0, 0);
  vertex.Tangent = { mesh->mTangents[i].x, mesh->mTangents[i].y,
                    mesh->mTangents[i].z };
  vertex.Bitangent = { mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                      mesh->mBitangents[i].z };
  if (is_load_armature) {
    _SetVertexBones(vertex, i, vertex_index_to_weights);
  }
  vertices.push_back(vertex);
}

void MeshManager::_SetVertexBones(
    Vertex& vertex, int i,
    unordered_map<unsigned int, set<pair<Bone*, aiVertexWeight>, weight_comparator>>&
        vertex_index_to_weights) {
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
}

void MeshManager::_SetBoneHierarchy(Armature* armature, aiNode* parent_node,
                                    Bone* parent, bool is_dae) {
  for (size_t j = 0; j < parent_node->mNumChildren; j++) {
    // if (j == 0) {
    //    parent->children.reserve(parent_node->mNumChildren);
    //}
    auto child_node = parent_node->mChildren[j];
    auto child_bone = armature->name_to_bone.find(
        GetBoneName(child_node->mName.C_Str(), armature, is_dae));
    if (child_bone != armature->name_to_bone.end()) {
      (*child_bone).second->parent = parent;
      parent->children.push_back((*child_bone).second);
      _SetBoneHierarchy(armature, child_node, (*child_bone).second);
    }
  }
}

MaterialTexNames MeshManager::_GetTexNames(const aiMesh* mesh,
                                            const aiScene* scene, bool is_obj) {
  string diffuse_name;
  string normal_name;
  string specular_name;
  string height_name;
  string metal_name;
  string ao_name;
  string opacity_name;

  aiString Path;
  auto mat = scene->mMaterials[mesh->mMaterialIndex];
  if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    diffuse_name = filesystem::path(Path.C_Str()).filename().string();
  }
  if (mat->GetTexture(aiTextureType_NORMALS, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    normal_name = filesystem::path(Path.C_Str()).filename().string();
  }
  if (mat->GetTexture(aiTextureType_SPECULAR, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    specular_name = filesystem::path(Path.C_Str()).filename().string();
  }
  if (mat->GetTexture(aiTextureType_HEIGHT, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    // Blender obj export saves .mtl with normal tex as bump tex.
    if (is_obj) {
      normal_name = filesystem::path(Path.C_Str()).filename().string();
    }
    else {
      height_name = filesystem::path(Path.C_Str()).filename().string();
    }
  }
  if (mat->GetTexture(aiTextureType_METALNESS, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    metal_name = filesystem::path(Path.C_Str()).filename().string();
  }
  if (mat->GetTexture(aiTextureType_AMBIENT_OCCLUSION, 0, &Path, NULL, NULL,
    NULL, NULL, NULL) == AI_SUCCESS) {
    ao_name = filesystem::path(Path.C_Str()).filename().string();
  }
  if (mat->GetTexture(aiTextureType_OPACITY, 0, &Path, NULL, NULL, NULL, NULL,
    NULL) == AI_SUCCESS) {
    opacity_name = filesystem::path(Path.C_Str()).filename().string();
  }
  return { diffuse_name, normal_name, specular_name, height_name,
                          metal_name, ao_name, opacity_name };
}

void MeshManager::_BlenderPostprocessLights(vector<Light*>& lights) {
  for (auto& light : lights) {
    light->constant_attenuation = 1;
    light->linear_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
    light->quadratic_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
    float denom = 10;
    light->color /= denom;
    light->specular /= denom;
  }
}

shared_ptr<MeshLoadData> MeshManager::CreateMesh(vector<float>& vertices,
                                                 vector<unsigned int>& indices) {
  vector<Vertex> positions;
  for (int i = 0; i < vertices.size(); i += 3) {
    Vertex vert;
    vert.Position = vec3(vertices[i], vertices[i + 1], vertices[i + 2]);
    positions.push_back(vert);
  }
  return CreateMesh(positions, indices, nullptr);
}
