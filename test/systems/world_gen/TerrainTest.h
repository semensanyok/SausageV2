#pragma once
#include <TerrainManager.h>
#include <GameLoop.h>

class TerrainTest {
  class TestScene : public Scene {
    vector<MeshData*> all_meshes;
    vector<MeshDataInstance*> all_meshes_instances;

    vector<MeshDataStatic*> all_static_meshes;
    vector<MeshDataInstance*> all_static_meshes_instances;

    vector<Light*> all_lights;
    vector<Light*> draw_lights;

    string scene_path = GetModelPath("Cube.001.fbx");

  public:
    SystemsManager* sm;
    void Init() {
      SausageGameLoop::Init();
      sm = SystemsManager::GetInstance();
      auto tm = sm->terrain_manager;
      tm->CreateTerrain(100, 100);

      _LoadMeshes(scene_path);
      _AddRigidBodies<MeshData, Vertex>(all_meshes);
    };
    void PrepareDraws() {
    };


    template<typename MESH_TYPE, typename VERTEX_TYPE>
    void _AddRigidBodies(vector<MESH_TYPE*>& new_meshes) {
      for (auto& mesh : new_meshes) {
        SausageUserPointer* up = nullptr;
        up = mesh;

        sm->physics_manager->AddBoxRigidBody(
          mesh->physics_data,
          new PhysicsTransformUpdateMesh<BlendTextures, MESH_TYPE, VERTEX_TYPE>(mesh),
          mesh->transform,
          mesh->name.c_str()
        );
      }
    }

    void _LoadMeshes(string& path) {
      vector<shared_ptr<MeshLoadData<Vertex>>> mesh_load_data_animated;
      vector<shared_ptr<MeshLoadData<VertexStatic>>> mesh_load_data_static;

      vector<MaterialTexNames> tex_names_list_animated;
      vector<MaterialTexNames> tex_names_list_static;

      sm->mesh_manager->LoadMeshes(path, all_lights,
        mesh_load_data_animated, mesh_load_data_static,
        tex_names_list_animated, tex_names_list_static,
        true, true, true);
      // SetBaseMeshForInstancedCommand
      SetupInstancedMeshStatic(mesh_load_data_static, tex_names_list_static);
      // LIGHTS SETUP
      draw_lights.insert(draw_lights.end(), all_lights.begin(), all_lights.end());
    }

    void SetupInstancedMeshStatic(std::vector<std::shared_ptr<MeshLoadData<VertexStatic>>>& mesh_load_data_animated,
      std::vector<MaterialTexNames>& tex_names_list_animated)
    {
      auto res = sm->mesh_data_utils->SetupInstancedMesh<MeshDataStatic, VertexStatic, BlendTextures>(
        sm->draw_call_manager->mesh_static_dc,
        mesh_load_data_animated, tex_names_list_animated);
      for (auto r : res) {
        all_static_meshes.push_back(r.mesh);
        for (auto i : r.instances) {
          all_static_meshes_instances.push_back(i);
        }
      }
    }
  };
public:
  static void run() {
    SausageGameLoop::run(new TestScene());
  };
};
