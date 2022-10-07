#pragma once

#include <FileWatcher.h>
#include <Settings.h>
#include <structures/Structures.h>
#include <sausage.h>
#include <ImguiGui.h>
#include <systems/MeshManager.h>
#include <systems/Renderer.h>
#include <systems/SystemsManager.h>
#include <systems/TextureManager.h>
#include <Scene.h>

using namespace std;

class Scene1 : public Scene {
 public:
  SystemsManager* systems_manager;

  // custom draws per shader
  vector<MeshDataBase*> all_meshes;
  vector<MeshDataBase*> all_transparent_meshes;

  vector<MeshDataBase*> draw_meshes;
  vector<MeshDataBase*> draw_transparent_meshes;

  vector<Light*> all_lights;
  vector<Light*> draw_lights;

  string scene_path = GetModelPath("Frog.fbx");
  // string scene_path = GetModelPath("dae/Frog.dae");
  // string scene_path = GetModelPath("Frog.gltf");

  set<string> clickable_meshes_names = {
    "Cube.001",
    "Cube.002",
    "Cube.003",
    "Cube.004",
    "Frog"
  };

  Scene1()
      : systems_manager{ SystemsManager::GetInstance() } {
    auto buffer = systems_manager->buffer_manager->mesh_data_buffer;

    // draw_call2 = new DrawCall(*draw_call);
    ////draw_call2->shader = shaders.stencil;
    // draw_call2->command_buffer =
    // draw_call2->buffer->CreateCommandBuffer(command_buffer_size);

    systems_manager->renderer->AddDraw(draw_call, DrawOrder::MESH);
     //systems_manager->renderer->AddDraw(draw_call2, DrawOrder::MESH);
  }
  ~Scene1(){};
  void Init() override {
    _LoadData();

    function<void()> scene_reload_callback = bind(&Scene1::_ReloadScene, this);
    bool is_persistent_command = false;
    scene_reload_callback =
        bind(&Renderer::AddGlCommand, systems_manager->renderer,
             scene_reload_callback, is_persistent_command);
    systems_manager->file_watcher->AddCallback(scene_path,
                                               scene_reload_callback);

    Gui::AddButton({"Reload scene", scene_reload_callback});
    CheckGLError();
  }
  void PrepareDraws() override {
    _OcclusionGather();
    _SortByDistance();
    vector<DrawElementsIndirectCommand> commands;
    for (int i = 0; i < draw_meshes.size(); i++) {
      if (draw_meshes[i]->base_mesh == nullptr) {
        commands.push_back(draw_meshes[i]->command);
      }
    }
    draw_call->commands_used = (unsigned int)commands.size();
    draw_call->buffer->AddCommands(commands, draw_call->command_buffer);
    CheckGLError();

    systems_manager->buffer_manager->mesh_data_buffer->BufferLights(draw_lights);
    CheckGLError();
  }

 private:
  void _LoadData() {
    vector<MeshDataBase*> new_mesh_data;
    vector<shared_ptr<MeshLoadData>> new_meshes;
    vector<Light*> new_lights;
    vector<MaterialTexNames> new_tex_names;
    _LoadGui();
    _LoadMeshes(scene_path, new_mesh_data, new_meshes, new_tex_names, new_lights);
    
    systems_manager->buffer_manager->mesh_data_buffer
        ->SetBaseMeshForInstancedCommand(new_mesh_data, new_meshes, new_tex_names);
    _BufferMeshes(new_mesh_data, new_meshes);
    for (auto mesh_base : new_mesh_data) {
      all_meshes.push_back(mesh_base);
    }
    for (auto& light : new_lights) {
      all_lights.push_back(light);
    }
    _AddRigidBodies(all_meshes);

    _LoadAnimations();
  }
  void _LoadMeshes(string& path, vector<MeshDataBase*>& out_new_mesh_data,
                   vector<shared_ptr<MeshLoadData>>& out_new_meshes,
                   vector<MaterialTexNames>& out_tex_names,
                   vector<Light*>& out_new_lights) {
    systems_manager->mesh_manager->LoadMeshes(path, out_new_lights,
      out_new_meshes, out_tex_names, true, true, true);
    for (auto& load_data : out_new_meshes) {
      auto mesh =
          systems_manager->mesh_manager->CreateMeshData(load_data.get());
      if (mesh->name != "Terrain") {
        mesh->physics_data->mass = 10.0;
      }
      mesh->physics_data->collision_group = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
      mesh->physics_data->collides_with_groups = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
      out_new_mesh_data.push_back(mesh);
    }
    CheckGLError();
  }
  void _LoadGui() {
    float font_scale = 0.1;
    auto trans = translate(rotate(scale(mat4(1.0), font_scale * vec3(1)), 90.0f, vec3(1,0,0)), vec3(0,10,0));
    string text1 = "Hola 3D!";
    string text2 = "Hola 2D!";

    //systems_manager->font_manager->WriteText3D(text1, {255.0, 0.0, 0.0}, trans);
    //systems_manager->font_manager->WriteTextUI(text2, {0.0, 255.0, 0.0}, 0, 0);
  }
  void _LoadAnimations() {
    for (auto mesh_base : all_meshes) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(mesh_base)) {
        if (mesh->armature != nullptr) {
          auto anim_mesh = systems_manager->anim_manager->CreateAnimMesh(mesh);
          systems_manager->anim_manager->LoadAnimationForMesh(scene_path, mesh);
          systems_manager->anim_manager->QueueMeshAnimUpdate(anim_mesh);
          if (!mesh->armature->name_to_anim.empty()) {
            auto anim1 = mesh->armature->name_to_anim["Stretch"];
            auto anim2 = mesh->armature->name_to_anim["ShakeHead"];
            auto anim3 = mesh->armature->name_to_anim["UpHead"];
            anim_mesh->AddAnim(AnimIndependentChannel::CHANNEL1, anim1);
            anim_mesh->AddAnim(AnimIndependentChannel::CHANNEL2, anim2);
            anim_mesh->AddAnim(AnimIndependentChannel::CHANNEL2, anim3);
          }
        }
      }
    }
    CheckGLError();
  }

  void _LoadTransparentMeshes(string& path,
                              vector<shared_ptr<MeshLoadData>>& out_new_meshes,
                              vector<Light*>& out_new_lights) {
    systems_manager->mesh_manager->LoadMeshes(path, out_new_lights,
                                              out_new_meshes);
    CheckGLError();
  }

  void _AddRigidBodies(vector<MeshDataBase*>& new_meshes) {
    for (auto& mesh_base : new_meshes) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(mesh_base)) {
        SausageUserPointer* up = nullptr;
        if (clickable_meshes_names.contains(mesh->name)) {
          up = new MeshDataClickable(mesh);
        } else {
          up = mesh;
        }
        systems_manager->physics_manager->AddBoxRigidBody(
          mesh->physics_data, up, mesh->transform, mesh->name
        );
      }
    }
  }

  void _BufferMeshes(vector<MeshDataBase*>& new_meshes_data,
                     vector<shared_ptr<MeshLoadData>>& new_meshes,
                     vector<MaterialTexNames>& tex_names) {
    CheckGLError();
    systems_manager->buffer_manager->mesh_data_buffer->BufferMeshData(
        new_meshes_data, new_meshes);
    CheckGLError();
    for (int i = 0; i < new_meshes_data.size(); i++) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(new_meshes_data[i])) {
        systems_manager->buffer_manager->mesh_data_buffer->BufferTransform(
            mesh);
      }
      if (MeshData* mesh = dynamic_cast<MeshData*>(new_meshes_data[i])) {
        Texture* texture = systems_manager->texture_manager->LoadTextureArray(
            tex_names[i]);
        if (texture != nullptr) {
          mesh->textures = { {1.0, texture->id }, 1 };
          systems_manager->buffer_manager->mesh_data_buffer->BufferMeshTexture(
              mesh);
          texture->MakeResident();
        }
      }
    }
  }

  void _ReloadScene() {
    lock_guard<mutex> pause_lock(Events::pause_phys_mtx);

    _CleanupScene();
    _LoadData();
  }

  void _CleanupScene() {
    systems_manager->Reset();
    all_meshes.clear();
    all_transparent_meshes.clear();
    all_lights.clear();
  }

  using distance_comparator =
      decltype([](const pair<float, MeshDataBase*>& lhs,
                  const pair<float, MeshDataBase*>& rhs) {
        return lhs.first > rhs.first;
      });

  void _SortByDistance() {
    set<pair<float, MeshDataBase*>, distance_comparator> back_to_front;
    for (auto mesh_base : draw_meshes) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(mesh_base)) {
        back_to_front.insert(
            {distance(systems_manager->camera->pos, vec3(mesh->transform[3])),
             mesh});
      }
    }
    draw_meshes.clear();
    for (auto& mesh_dist : back_to_front) {
      draw_meshes.push_back(mesh_dist.second);
    }
  }

  void _LoadTerrain() {}

  void _OcclusionGather() {
    draw_lights = all_lights;
    draw_meshes = all_meshes;
  }
};
