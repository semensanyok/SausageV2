#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>
#include <LightStruct.h>
#include <BufferManager.h>
#include <BoundingVolume.h>
#include <MeshDataUtils.h>

class TerrainTest : public Scene {
  vector<MeshData*> all_meshes;
  vector<MeshDataInstanceT*> all_meshes_instances;

  vector<MeshDataStatic*> all_static_meshes;
  vector<MeshDataInstanceStaticT*> all_static_meshes_instances;

  vector<Light*> all_lights;
  vector<Light*> draw_lights;

  string scene_path = GetModelPath("Cube.001.fbx");

  SystemsManager* sm;
public:
  TerrainTest(vec3 world_extents, unsigned int octree_num_levels) :
    Scene(world_extents, octree_num_levels), sm { SystemsManager::GetInstance() } {}
  void Init() {
    sm = SystemsManager::GetInstance();
    auto tm = sm->terrain_manager;
    auto size = 256;
    // center to world origin
    tm->CreateTerrain(size, size, vec3(0, 0, 0), 1);

    unordered_map<size_t, Texture*> base_meshes_tex;
    vector<shared_ptr<MeshLoadData<Vertex>>> mesh_load_data_animated;
    vector<shared_ptr<MeshLoadData<VertexStatic>>> mesh_load_data_static;

    _LoadMeshes(scene_path, mesh_load_data_animated, mesh_load_data_static, base_meshes_tex);
    // multiply instances
    auto mesh_static_buffer = sm->buffer_manager->GetMeshDataBufferConsumer<MeshDataStatic, VertexStatic, UniformDataMeshStatic>();
    auto mesh = all_static_meshes_instances[0];
    BlendTextures tex = { { 1.0, base_meshes_tex[mesh->base_mesh->id]->id },1 };
    for (size_t i = 0; i < 100; i++)
    {
      int spread = 2;
      mat4 t1 = glm::translate(all_static_meshes_instances[0]->ReadTransform(),
          i % 2 == 0 ? vec3((i % 10) * spread, 100, (i % 10) * spread + 2) : -vec3((i % 10) * spread, -100, (i % 3) * spread + 2));
      auto inst = new MeshDataInstanceStaticT(
        t1,
        mesh->base_mesh,
        new BoundingBox(t1, mesh->bv->min_AABB, mesh->bv->max_AABB));
      inst->physics_data = mesh->physics_data;
      inst->AllocateUniformOffset();
      inst->IncNumInstancesSetInstanceId();
      // TODO: texture slot to instance or separate class
      mesh_static_buffer->BufferMeshDataInstance(inst, tex);
      all_static_meshes_instances.push_back(inst);
    }

    for (auto mesh : all_static_meshes_instances)
    {
      sm->mesh_data_utils->AddRigidBody<MeshDataStatic, VertexStatic>(mesh);
    }

    auto pos = vec3(all_static_meshes_instances[0]->ReadTransform()[3] + vec4(0, 15, 15, 0));
    sm->camera->SetPosition(pos);
    DebugDrawOctree();
  };
  void PrepareDraws() {
  };

  void _LoadMeshes(string& path,
    unordered_map<size_t, Texture*>& base_meshes_tex) {
    vector<shared_ptr<MeshLoadData<Vertex>>> mesh_load_data_animated;
    vector<shared_ptr<MeshLoadData<VertexStatic>>> mesh_load_data_static;
    _LoadMeshes(path, mesh_load_data_animated, mesh_load_data_static,
      base_meshes_tex);
  }
  void _LoadMeshes(string& path,
    vector<shared_ptr<MeshLoadData<Vertex>>>& mesh_load_data_animated,
    vector<shared_ptr<MeshLoadData<VertexStatic>>>& mesh_load_data_static,
    unordered_map<size_t, Texture*>& base_meshes_tex
    ) {

    vector<MaterialTexNames> tex_names_list_animated;
    vector<MaterialTexNames> tex_names_list_static;
    sm->mesh_manager->LoadMeshes(path, all_lights,
      mesh_load_data_animated, mesh_load_data_static,
      tex_names_list_animated, tex_names_list_static,
      true, true, true);
    // SetBaseMeshForInstancedCommand
    SetupInstancedMeshStatic(mesh_load_data_static, tex_names_list_static, base_meshes_tex);
    // LIGHTS SETUP
    draw_lights.insert(draw_lights.end(), all_lights.begin(), all_lights.end());

    draw_lights.push_back(new Light{ vec4{-5, 5,5,0},
                                     vec4{2,2,2,0},
                                     vec4{22,22,22,0},//vec4{22,22,22,0},
                                     vec4{22,22,22,0},
                   LightType::Point,
                   0,
                   0,
                   1,//1,

                   0.7,//0.7,
                   0.7//0.7
      });

    //draw_lights.push_back(new Light{ vec4{5, 5,5,0},
    //                           vec4{2,20,2,0},
    //                           vec4{22,22,22,0},//vec4{22,22,22,0},
    //                           vec4{22,22,22,0},
    //         LightType::Spot,
    //         0,
    //         0,
    //         0.1,//1,

    //         0.07,//0.7,
    //         0.07//0.7
    //  });
    BufferStorage::GetInstance()->BufferLights(draw_lights);
  }

  void SetupInstancedMeshStatic(std::vector<std::shared_ptr<MeshLoadData<VertexStatic>>>& mesh_load_data_animated,
    std::vector<MaterialTexNames>& tex_names_list,
    unordered_map<size_t, Texture*>& base_meshes_tex)
  {
    vector<SetupInstancedMeshResStaticT> res = sm->mesh_data_utils->SetupInstancedMesh<MeshDataStatic, VertexStatic, BlendTextures, UniformDataMeshStatic>(
      sm->draw_call_manager->mesh_static_dc,
      mesh_load_data_animated,
      tex_names_list,
      base_meshes_tex);
    for (auto r : res) {
      all_static_meshes.push_back(r.mesh);
      for (auto i : r.instances) {
        all_static_meshes_instances.push_back(i);
      }
    }
  }
};
