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
#include "Physics.h"
#include "MeshDataInstance.h"

using namespace std;

template<typename MESH_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
struct SetupInstancedMeshRes {
  shared_ptr<MeshLoadData<VERTEX_TYPE>> load_data;
  MESH_TYPE* mesh;
  vector<MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>*> instances;
};
using SetupInstancedMeshResT = SetupInstancedMeshRes<MeshData, Vertex, UniformDataMesh>;
using SetupInstancedMeshResStaticT = SetupInstancedMeshRes<MeshDataStatic, VertexStatic, UniformDataMeshStatic>;

class MeshDataUtils {

  DrawCallManager* draw_call_manager;
  TextureManager* texture_manager;
  MeshManager* mesh_manager;
  BufferManager* buffer_manager;
  PhysicsManager* physics_manager;
public:
  MeshDataUtils(
    DrawCallManager* draw_call_manager,
    TextureManager* texture_manager,
    MeshManager* mesh_manager,
    BufferManager* buffer_manager,
    PhysicsManager* physics_manager
  ) : draw_call_manager{ draw_call_manager },
    texture_manager{ texture_manager },
    mesh_manager{ mesh_manager },
    buffer_manager{ buffer_manager },
    physics_manager{ physics_manager } {};

  /**
   * @return mesh data with instance_count
  */
  template<typename MESH_TYPE, typename VERTEX_TYPE, typename TEXTURE_ARRAY_TYPE, typename UNIFORM_DATA_TYPE>
  vector<SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>> SetupInstancedMesh(
    DrawCall* dc,
    std::vector<std::shared_ptr<MeshLoadData<VERTEX_TYPE>>>& mesh_load,
    std::vector<MaterialTexNames>& tex_names_list,
    unordered_map<size_t, Texture*>& out_base_meshes_tex) {
    vector<SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>> res;
    unordered_map<size_t, vector<shared_ptr<MeshLoadData<VERTEX_TYPE>>>> base_meshes;

    MeshDataBufferConsumerShared<TEXTURE_ARRAY_TYPE, MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>* buffer =
      buffer_manager->GetMeshDataBufferConsumer<MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>();

    hash<MaterialTexNames> tex_hash;
    unordered_map<size_t, Texture*> tex_by_hash;

    PhysicsData* common_physics_data = new PhysicsData();
    common_physics_data->mass = 10.0;
    common_physics_data->collision_group = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
    common_physics_data->collides_with_groups = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;

    for (int i = 0; i < mesh_load.size(); i++) {
      auto load_data_sptr = mesh_load[i];
      auto load_data = load_data_sptr.get();
      auto& tex_names = tex_names_list[i];
      auto key =
        tex_hash(tex_names)
        + load_data->vertices.size()
        + load_data->indices.size();



      if (!base_meshes.contains(key)) {
        base_meshes[key] = { { load_data_sptr } };
        Texture* texture = texture_manager->LoadTextureArray(tex_names);
        // TEXTURE SETUP
        if (texture != nullptr) {
          texture->MakeResident();
        }
        tex_by_hash[key] = texture;
      }
      else {
        base_meshes[key].push_back(load_data_sptr);
      }
    }
    for (auto& mesh_instances : base_meshes) {
      size_t key = mesh_instances.first;
      auto& tex = tex_by_hash[key];
      auto& instances = mesh_instances.second;

      shared_ptr<MeshLoadData<VERTEX_TYPE>>& base_ptr = instances[0];
      auto base = base_ptr.get();
      // BASE MESH SETUP
      MESH_TYPE* mesh = mesh_manager->CreateMeshData<VERTEX_TYPE, MESH_TYPE>(base_ptr);
      out_base_meshes_tex[mesh->id] = tex;
      res.push_back({ base_ptr, mesh, {} });
      SetupInstancedMeshRes<MESH_TYPE, VERTEX_TYPE, UNIFORM_DATA_TYPE>& mesh_res = res[res.size() - 1];

      if (!buffer->AllocateStorage(mesh->slots, base_ptr->vertices.size(), base_ptr->indices.size())) {
        // logged in vertex_attributes AllocateStorage
        continue;
      };
      buffer->BufferMeshData(mesh, base_ptr);

      // INSTANCES SETUP
      for (int i = 0; i < instances.size(); i++) {
        shared_ptr<MeshLoadData<VERTEX_TYPE>>& idata = instances[i];
        // already have set correct instance_count, no need to update (via AddNewCommandToDrawCall<MESH_TYPE>(mesh, dc, instances.size()))
        MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>* mesh_instance = new MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>(idata->transform, mesh,
          idata->bv);
        buffer->AllocateUniformOffset(mesh_instance);
        // TEXTURE SETUP
        BlendTextures btex = { { 1.0, tex->id }, 1 };
        buffer->BufferMeshDataInstance(mesh_instance, btex);
        // PHYSICS SETUP
        mesh_instance->physics_data = common_physics_data;
        mesh_res.instances.push_back(mesh_instance);
      }
    }
    return res;
  }

  vector<SetupInstancedMeshRes<MeshData, Vertex, UniformDataMesh>> SetupInstancedMeshWithAnim(
    DrawCall* dc,
    std::vector<std::shared_ptr<MeshLoadData<Vertex>>>& mesh_load_data_animated,
    std::vector<MaterialTexNames>& tex_names_list,
    unordered_map<size_t, Texture*>& base_meshes_tex)
  {
    auto res = SetupInstancedMesh<MeshData, Vertex, BlendTextures, UniformDataMesh>(dc, mesh_load_data_animated, tex_names_list, base_meshes_tex);

    for (auto& mesh_res : res) {
      auto armature = mesh_res.load_data->armature;
      mesh_res.mesh->armature = armature;
    }
    return res;
  }

  /**
   * @tparam MESH_TYPE
   * @tparam VERTEX_TYPE
   * @param new_meshes
   * @param custom_up custom user pointer for physics subsystem. i.e. MeshDataClickable
  */
  template<typename MESH_TYPE, typename VERTEX_TYPE, typename UNIFORM_DATA_TYPE>
  void AddRigidBody(MeshDataInstance<MESH_TYPE, UNIFORM_DATA_TYPE>* mesh
    // TODO: support second callback for MeshDataCickable
    //,SausageUserPointer* custom_up = nullptr
  ) {
    physics_manager->AddBoxRigidBody(
      mesh->physics_data,
      mesh->bv,
      mesh,
      mesh->name.c_str()
    );
  }
};

