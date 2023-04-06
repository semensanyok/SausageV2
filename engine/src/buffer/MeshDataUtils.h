#pragma once

#include "sausage.h"
#include "MeshDataStruct.h"
#include "DrawCallManager.h"
#include "TextureManager.h"
#include "MeshManager.h"
#include "AnimationStruct.h"
#include "GLVertexAttributes.h"
#include "MeshDataBufferConsumerShared.h"
#include "PhysicsStruct.h"

using namespace std;

class MeshDataUtils {

  DrawCallManager* draw_call_manager;
  TextureManager* texture_manager;
  MeshManager* mesh_manager;
  BufferManager* buffer_manager;
public:
  MeshDataUtils(
    DrawCallManager* draw_call_manager,
    TextureManager* texture_manager,
    MeshManager* mesh_manager,
    BufferManager* buffer_manager
  ) : draw_call_manager{ draw_call_manager },
    texture_manager{ texture_manager },
    mesh_manager{ mesh_manager },
    buffer_manager{ buffer_manager } {};

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
    std::vector<std::shared_ptr<MeshLoadData<VERTEX_TYPE>>>& mesh_load,
    std::vector<MaterialTexNames>& tex_names_list_animated)
  {
    vector<SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE>> res;
    unordered_map<size_t, pair<MaterialTexNames, vector<shared_ptr<MeshLoadData<VERTEX_TYPE>>>>> base_meshes;
    hash<MaterialTexNames> tex_hash;
    for (int i = 0; i < mesh_load.size(); i++) {
      auto load_data_sptr = mesh_load[i];
      auto load_data = load_data_sptr.get();
      auto tex_names = tex_names_list_animated[i];
      auto key =
        tex_hash(tex_names)
        + load_data->vertices.size()
        + load_data->indices.size();

      if (!base_meshes.contains(key)) {
        base_meshes[key] = { tex_names ,{ load_data_sptr } };
      }
      else {
        base_meshes[key].second.push_back(load_data_sptr);
      }
    }
    for (auto& mesh_instances : base_meshes) {
      auto& tex_names = mesh_instances.second.first;
      auto& instances = mesh_instances.second.second;

      shared_ptr<MeshLoadData<VERTEX_TYPE>>& base_ptr = instances[0];
      auto base = base_ptr.get();
      // BASE MESH SETUP
      MESH_TYPE* mesh = mesh_manager->CreateMeshData<VERTEX_TYPE, MESH_TYPE>(base_ptr);
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
      auto mesh_data_buffer_consumer = buffer_manager->GetBuffer<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>();
      if (!mesh_data_buffer_consumer->AllocateStorage(mesh->slots, base_ptr->vertices.size(), base_ptr->indices.size())) {
        // logged in vertex_attributes AllocateStorage
        continue;
      };
      draw_call_manager->AddNewCommandToDrawCall<MESH_TYPE>(mesh, dc, instances.size());
      mesh_data_buffer_consumer->BufferMeshData(mesh, base_ptr);

      // PHYSICS SETUP
      {
        mesh->physics_data = base->physics_data;
        if (base->name != "Terrain") {
          mesh->physics_data->mass = 10.0;
        }
        else {
          mesh->physics_data->mass = 0.0;
        }
        mesh->physics_data->collision_group = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
        mesh->physics_data->collides_with_groups = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
      }

      // INSTANCES SETUP
      for (int i = 1; i < instances.size(); i++) {
        shared_ptr<MeshLoadData<VERTEX_TYPE>>& idata = instances[i];
        // already have set correct instance_count, no need to update (via AddNewCommandToDrawCall<MESH_TYPE>(mesh, dc, instances.size()))
        MeshDataInstance* mesh_instance = draw_call_manager->AddNewInstance<MESH_TYPE>(mesh, idata->transform);
        mesh_data_buffer_consumer->BufferMeshDataInstance(mesh_instance, mesh->textures);
        mesh_data_buffer_consumer->BufferTexture(mesh_instance, mesh->textures);
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

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename VERTEX_TYPE>
class PhysicsTransformUpdateMesh : public PhysicsTransformUpdate {
  MESH_TYPE* mesh;
  MeshDataBufferConsumerShared<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>* buffer;
public:
  PhysicsTransformUpdateMesh(MESH_TYPE* mesh) :
    mesh{ mesh },
    buffer{ buffer } {};
  mat4& GetOutMatrix() override {
    return mesh->transform;
  };
  void OnTransformUpdate() override {
    BufferManager::GetInstance()->
      GetBuffer<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>()->BufferTransform(mesh, mesh->transform);
  };
};

template<typename TEXTURE_ARRAY_TYPE, typename MESH_TYPE, typename VERTEX_TYPE>
class PhysicsTransformUpdateMeshInstance : public PhysicsTransformUpdate {
  MeshDataInstance* mesh;
  MeshDataBufferConsumerShared<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>* buffer;
public:
  PhysicsTransformUpdateMeshInstance(MeshDataInstance* mesh) :
    mesh{ mesh },
    buffer{ buffer } {};
  mat4& GetOutMatrix() override {
    return mesh->transform;
  };
  void OnTransformUpdate() override {
    BufferManager::GetInstance()->
      GetBuffer<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE>()->BufferTransform(mesh, mesh->transform);
  };
};
