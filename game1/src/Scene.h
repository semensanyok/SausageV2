#pragma once

#include "FileWatcher.h"
#include "Settings.h"
#include "Structures.h"
#include "buffer.h"
#include "sausage.h"
#include "systems/Gui.h"
#include "systems/MeshManager.h"
#include "systems/Renderer.h"
#include "systems/SystemsManager.h"
#include "systems/TextureManager.h"

using namespace std;

class Scene {
 public:
  SystemsManager* systems_manager;

  DrawCall* draw_call;

  Shaders* shaders;
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
  Scene(SystemsManager* systems_manager)
      : systems_manager{systems_manager}, shaders{systems_manager->shaders} {
    draw_call = new DrawCall();
    draw_call->shader = shaders->blinn_phong;
    draw_call->mode = GL_TRIANGLES;
    draw_call->buffer = systems_manager->buffer_manager->mesh_data_buffer;
    draw_call->command_buffer =
        draw_call->buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND);
    draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);

    // draw_call2 = new DrawCall(*draw_call);
    ////draw_call2->shader = shaders.stencil;
    // draw_call2->command_buffer =
    // draw_call2->buffer->CreateCommandBuffer(command_buffer_size);

    systems_manager->renderer->AddDraw(draw_call);
    // systems_manager->renderer->AddDraw(draw_call2);
  }
  ~Scene(){};
  void Init() {
    _LoadData();

    function<void()> scene_reload_callback = bind(&Scene::_ReloadScene, this);
    bool is_persistent_command = false;
    scene_reload_callback =
        bind(&Renderer::AddGlCommand, systems_manager->renderer,
             scene_reload_callback, is_persistent_command);
    systems_manager->file_watcher->AddCallback(scene_path,
                                               scene_reload_callback);

    Gui::AddButton({"Reload scene", scene_reload_callback});
    CheckGLError();
  }
  void PrepareDraws() {
    _OcclusionGather();
    _SortByDistance();
    vector<DrawElementsIndirectCommand> commands;
    for (int i = 0; i < draw_meshes.size(); i++) {
      if (draw_meshes[i]->base_mesh == nullptr) {
        commands.push_back(draw_meshes[i]->command);
      }
    }
    draw_call->buffer->AddCommands(commands, draw_call->command_buffer);
    CheckGLError();
    draw_call->command_count = (unsigned int)commands.size();
    draw_call->num_lights = (int)draw_lights.size();

    systems_manager->buffer_manager->mesh_data_buffer->BufferLights(
        draw_lights);
    CheckGLError();
  }

 private:
  void _LoadData() {
    vector<MeshDataBase*> new_mesh_data;
    vector<shared_ptr<MeshLoadData>> new_meshes;
    vector<Light*> new_lights;
    _LoadMeshes(scene_path, new_mesh_data, new_meshes, new_lights);

    systems_manager->buffer_manager->mesh_data_buffer
        ->SetBaseMeshForInstancedCommand(new_mesh_data, new_meshes);
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
                   vector<Light*>& out_new_lights) {
    systems_manager->mesh_manager->LoadMeshes(path, out_new_lights,
                                              out_new_meshes, true, true, true);
    for (auto& load_data : out_new_meshes) {
      auto mesh = systems_manager->mesh_manager->CreateMeshData(load_data.get());
      if (mesh->name != "Terrain") {
        mesh->physics_data->mass = 10.0;
      }
      out_new_mesh_data.push_back(mesh);
    }
    CheckGLError();
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
        systems_manager->physics_manager->AddBoxRigidBody(
            mesh->physics_data, mesh, mesh->transform);
      }
    }
  }
  void _BufferMeshes(vector<MeshDataBase*>& new_meshes_data,
    vector<shared_ptr<MeshLoadData>>& new_meshes) {
    for (int i = 0; i < new_meshes.size();  i++) {
      auto load_data = new_meshes[i].get();
      // dae only imports albedo name. temporary fix. TODO: delete
      {
        if (load_data->tex_names.normal.empty()) {
          load_data->tex_names.normal = load_data->tex_names.diffuse;
          load_data->tex_names.specular = load_data->tex_names.diffuse;
          auto u = load_data->tex_names.diffuse.find("_");
          auto to_replace = load_data->tex_names.diffuse.substr(
              u + 1, load_data->tex_names.diffuse.find(".") - u - 1);
          load_data->tex_names.normal.replace(u + 1, to_replace.size(), "normal");
          load_data->tex_names.specular.replace(u + 1, to_replace.size(),
                                           "specular");
        }
      }
    }
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
        mesh->texture = systems_manager->texture_manager->LoadTextureArray(
            new_meshes[i]->tex_names);
        if (mesh->texture != nullptr) {
          systems_manager->buffer_manager->mesh_data_buffer->BufferMeshTexture(mesh);
          mesh->texture->MakeResident();
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
  using distance_comparator = decltype([](const pair<float, MeshDataBase*>& lhs,
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
