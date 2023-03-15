#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "DrawCallManager.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "AnimationStruct.h"
#include "GLVertexAttributes.h"

using namespace std;

class MeshDataUtils {

  DrawCallManager* draw_call_manager;
  TextureManager* texture_manager;
  MeshManager* mesh_manager;
  BufferStorage* buffer_storage;
  GLVertexAttributes* vertex_attributes;
public:
  MeshDataUtils(
    DrawCallManager* draw_call_manager,
    TextureManager* texture_manager,
    MeshManager* mesh_manager,
    GLVertexAttributes* vertex_attributes
  ) : draw_call_manager{ draw_call_manager },
    texture_manager{ texture_manager },
    mesh_manager{ mesh_manager },
    vertex_attributes{ vertex_attributes },
    buffer_storage{ BufferStorage::GetInstance() } {};

  template<typename MESH_TYPE, typename VERTEX_TYPE>
  struct SetupInstancedMeshRes {
    shared_ptr<MeshLoadData<VERTEX_TYPE>> load_data;
    MESH_TYPE* mesh;
    vector<MeshDataInstance*> instances;
  };

  /**
   * @return mesh data with instance_count
  */
  template<typename MESH_TYPE, typename VERTEX_TYPE, typename TEXTURE_ARRAY_TYPE>
  vector<SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE>> SetupInstancedMesh(
    DrawCall* dc,
    std::vector<std::shared_ptr<MeshLoadData<VERTEX_TYPE>>>& mesh_load_data_animated,
    std::vector<MaterialTexNames>& tex_names_list_animated)
  {
    vector<SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE>> res;
    unordered_map<size_t, pair<MaterialTexNames, vector<shared_ptr<MeshLoadData<VERTEX_TYPE>>>>> base_meshes;
    hash<MaterialTexNames> tex_hash;
    for (int i = 0; i < mesh_load_data_animated.size(); i++) {
      auto load_data_sptr = mesh_load_data_animated[i];
      auto load_data = load_data_sptr.get();
      auto tex_names = tex_names_list_animated[i];
      auto key =
        tex_hash(tex_names)
        + load_data->vertices.size()
        + load_data->indices.size();

      if (!base_meshes.contains(key)) {
        base_meshes[key] = { tex_names ,{} };
      }
      else {
        base_meshes[key].second.push_back(load_data_sptr);
      }
    }
    for (auto& mesh_instances : base_meshes) {
      auto& tex_names = mesh_instances.second.first;
      auto& instances = mesh_instances.second.second;
      if (instances.empty()) {
        continue;
      }

      shared_ptr<MeshLoadData<VERTEX_TYPE>>& base_ptr = instances[0];
      auto base = base_ptr.get();
      // BASE MESH SETUP
      MESH_TYPE* mesh = mesh_manager->CreateMeshData<VERTEX_TYPE, MESH_TYPE>(base_ptr.get());
      SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE> mesh_res = { base_ptr, mesh, {} };
      res.push_back(mesh_res);
      // TEXTURE SETUP
      {
        Texture* texture = texture_manager->LoadTextureArray(tex_names);
        if (texture != nullptr) {
          mesh->textures = { { 1.0, texture->id }, 1 };
          texture->MakeResident();
        }
      }
      vertex_attributes->BufferVertices<VERTEX_TYPE>(mesh->slots, base_ptr);
      draw_call_manager->AddNewCommandToDrawCall<MESH_TYPE>(mesh, dc, instances.size());

      // PHYSICS SETUP
      {
        if (base->name != "Terrain") {
          mesh->physics_data = base->physics_data;
          mesh->physics_data->mass = 10.0;
        }
        mesh->physics_data->collision_group = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
        mesh->physics_data->collides_with_groups = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
      }

      // INSTANCES SETUP
      for (shared_ptr<MeshLoadData<VERTEX_TYPE>>& idata : instances) {
        MeshDataInstance* mesh_instance = draw_call_manager->AddNewInstance<MESH_TYPE>(mesh, idata->transform);
        buffer_storage->BufferTransform<mat4, MESH_TYPE>(mesh_instance, mesh_instance->transform);
        // TEXTURE SETUP
        buffer_storage->BufferTexture<MESH_TYPE, TEXTURE_ARRAY_TYPE>(mesh_instance, mesh->textures);
        mesh_res.instances.push_back(mesh_instance);
      }
    }
    return res;
  }

  vector<SetupInstancedMeshRes<MeshData, Vertex>> SetupInstancedMeshWithAnim(
    DrawCall* dc,
    std::vector<std::shared_ptr<MeshLoadData<Vertex>>>& mesh_load_data_animated,
    std::vector<MaterialTexNames>& tex_names_list_animated)
  {
    auto res = SetupInstancedMesh<MeshData, Vertex, BlendTextures>(dc, mesh_load_data_animated, tex_names_list_animated);

    for (auto mesh_res : res) {
      auto armature = mesh_res.load_data->armature;
      mesh_res.mesh->armature = armature;
    }
    return res;
  }
};
