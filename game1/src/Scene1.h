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
  DrawCall* mesh_dc;
  DrawCallManager* draw_call_manager;
  MeshManager* mesh_manager;
  MeshDataBufferConsumer* mesh_data_buffer;
  MeshStaticBufferConsumer* mesh_static_buffer;

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
    : systems_manager{ SystemsManager::GetInstance() },
    draw_call_manager{ systems_manager->draw_call_manager },
    mesh_dc{ systems_manager->draw_call_manager->mesh_dc },
    mesh_data_buffer{ systems_manager->buffer_manager->mesh_data_buffer },
    mesh_manager{ systems_manager->mesh_manager }{
  }
  ~Scene1() {};
  void Init() override {
    _LoadData();

    function<void()> scene_reload_callback = bind(&Scene1::_ReloadScene, this);
    bool is_persistent_command = false;
    scene_reload_callback =
      bind(&Renderer::AddGlCommand, systems_manager->renderer,
           scene_reload_callback, is_persistent_command);
    systems_manager->file_watcher->AddCallback(scene_path,
                                               scene_reload_callback);

    Gui::AddButton({ "Reload scene", scene_reload_callback });
    CheckGLError();
  }
  void PrepareFrameDraws() override {
    // need to implement order independent transparency to not touch command buffer each frame
    //_SortByDistance();

    systems_manager->buffer_manager->storage->BufferLights(draw_lights);
    CheckGLError();
  }

private:
  void _LoadData() {
    vector<Light*> new_lights;
    _LoadGui();
    _LoadMeshes(scene_path);

    _AddRigidBodies(all_meshes);
    _LoadAnimations();
  }

  void _LoadMeshes(string& path) {
    vector<shared_ptr<MeshLoadData<Vertex>>> mesh_load_data_animated;
    vector<shared_ptr<MeshLoadData<VertexStatic>>> mesh_load_data_static;

    vector<MaterialTexNames> tex_names_list_animated;
    vector<MaterialTexNames> tex_names_list_static;

    systems_manager->mesh_manager->LoadMeshes(path, all_lights,
      mesh_load_data_animated, mesh_load_data_static,
      tex_names_list_animated, tex_names_list_static,
      true, true, true);

    // SetBaseMeshForInstancedCommand
    unordered_map<size_t, pair<MaterialTexNames, vector<shared_ptr<MeshLoadData<Vertex>>>>> base_meshes;
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
        base_meshes[key] = { tex_names , {} };
      }
      else {
        base_meshes[key].second.push_back(load_data_sptr);
      }
    }
    for (auto& mesh_instances : base_meshes) {
      auto& tex_names = mesh_instances.second.first;
      auto& instances = mesh_instances.second.second;

      auto base_ptr = instances[0];
      auto base = base_ptr.get();
      // BASE MESH SETUP
      auto mesh = mesh_manager->CreateMeshData<Vertex>(base);
      if (!mesh_data_buffer->AllocateStorage(mesh->slots,
        base->vertices.size(), base->indices.size())) {
        throw runtime_error("Scene1: failed to Allocate");
      }
      mesh_data_buffer->BufferVertices(mesh->slots, base_ptr);
      draw_call_manager->AddNewCommandToDrawCall<MeshData>(mesh, mesh_dc, instances.size());
      mesh_data_buffer->BufferTransform(mesh, mesh->transform);
      // TEXTURE SETUP
      {
        Texture* texture = systems_manager->texture_manager->LoadTextureArray(tex_names);
        if (texture != nullptr) {
          mesh->textures = { {1.0, texture->id }, 1 };
          systems_manager->buffer_manager->mesh_data_buffer->BufferTexture(
              mesh, mesh->textures);
          texture->MakeResident();
        }
      }
      // PHYSICS SETUP
      {
        if (base->name != "Terrain") {
          mesh->physics_data = base->physics_data;
          mesh->physics_data->mass = 10.0;
        }
        mesh->physics_data->collision_group = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
        mesh->physics_data->collides_with_groups = SausageCollisionMasks::MESH_GROUP_0 | SausageCollisionMasks::CLICKABLE_GROUP_0;
      }
      mesh->armature = base->armature;
      all_meshes.push_back(mesh);

      // INSTANCES SETUP  
      for (auto& idata : instances) {
        MeshDataInstance* instance = draw_call_manager->AddNewInstance<MeshData>(mesh, base->transform);
        mesh_data_buffer->BufferTransform(instance, instance->transform);
        // TEXTURE SETUP
        mesh_data_buffer->BufferTexture(instance, mesh->textures);
      }
    }
    // LIGHTS SETUP
    draw_lights.insert(draw_lights.end(), all_lights.begin(), all_lights.end());
  }

  void _LoadGui() {
    float font_scale = 0.1;
    auto trans = translate(rotate(scale(mat4(1.0), font_scale * vec3(1)), 90.0f, vec3(1, 0, 0)), vec3(0, 10, 0));
    string text1 = "Hola 3D!";
    string text2 = "Hola 2D!";

    //systems_manager->font_manager->WriteText3D(text1, {255.0, 0.0, 0.0}, trans);
    //systems_manager->font_manager->WriteTextUI(text2, {0.0, 255.0, 0.0}, 0, 0);
  }
  void _LoadAnimations() {
    for (auto mesh_base : all_meshes) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(mesh_base)) {
        if (mesh->armature != nullptr) {
          auto active_anim = systems_manager->anim_manager->CreateActiveAnimInstance(mesh->armature);
          systems_manager->anim_manager->LoadAnimationForArmature(scene_path, mesh->armature);
          systems_manager->anim_manager->QueueAnimUpdate(active_anim);
          if (!mesh->armature->name_to_anim.empty()) {
            auto anim1 = mesh->armature->name_to_anim["Stretch"];
            auto anim2 = mesh->armature->name_to_anim["ShakeHead"];
            auto anim3 = mesh->armature->name_to_anim["UpHead"];
            active_anim->AddAnim(AnimIndependentChannel::CHANNEL1, anim1);
            active_anim->AddAnim(AnimIndependentChannel::CHANNEL2, anim2);
            active_anim->AddAnim(AnimIndependentChannel::CHANNEL2, anim3);
          }
        }
      }
    }
    CheckGLError();
  }

  void _AddRigidBodies(vector<MeshDataBase*>& new_meshes) {
    for (auto& mesh_base : new_meshes) {
      if (MeshData* mesh = dynamic_cast<MeshData*>(mesh_base)) {
        SausageUserPointer* up = nullptr;
        if (clickable_meshes_names.contains(mesh->name)) {
          up = new MeshDataClickable(mesh);
        }
        else {
          up = mesh;
        }
        systems_manager->physics_manager->AddBoxRigidBody(
          mesh->physics_data, up, mesh->transform, mesh->name
        );
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
            { distance(systems_manager->camera->pos, vec3(mesh->transform[3])),
             mesh });
      }
    }
    draw_meshes.clear();
    for (auto& mesh_dist : back_to_front) {
      draw_meshes.push_back(mesh_dist.second);
    }
  }
};
