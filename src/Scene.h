#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "systems/Renderer.h"
#include "Structures.h"
#include "systems/MeshManager.h"
#include "systems/TextureManager.h"
#include "FileWatcher.h"
#include "systems/SystemsManager.h"
#include "Settings.h"
#include "systems/Gui.h"

using namespace std;

class Scene {
public:
	SystemsManager* systems_manager;

	DrawCall* draw_call;

	Shaders shaders;
	// custom draws per shader
	vector<MeshData*> all_meshes;
	vector<MeshData*> all_transparent_meshes;

	vector<MeshData*> draw_meshes;
	vector<MeshData*> draw_transparent_meshes;

	vector<Light*> all_lights;
	vector<Light*> draw_lights;

	string scene_path = GetModelPath("frog.fbx");
	Scene(SystemsManager* systems_manager) :
		systems_manager{ systems_manager }, shaders{ systems_manager->shaders }{
		draw_call = new DrawCall();
		draw_call->shader = shaders.blinn_phong;
		draw_call->mode = GL_TRIANGLES;
		draw_call->buffer = systems_manager->buffer;
		draw_call->command_buffer = draw_call->buffer->CreateCommandBuffer(BufferSettings::MAX_COMMAND);
		draw_call->buffer->ActivateCommandBuffer(draw_call->command_buffer);

		//draw_call2 = new DrawCall(*draw_call);
		////draw_call2->shader = shaders.stencil;
		//draw_call2->command_buffer = draw_call2->buffer->CreateCommandBuffer(command_buffer_size);

		systems_manager->renderer->AddDraw(draw_call);
		//systems_manager->renderer->AddDraw(draw_call2);
	}
	~Scene() {};
	void Init() {
		_LoadData();

		function<void()> scene_reload_callback = bind(&Scene::_ReloadScene, this);
		systems_manager->file_watcher->AddCallback(scene_path, scene_reload_callback);

		Gui::AddButton({ "Reload scene", scene_reload_callback });
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

		systems_manager->buffer->BufferLights(draw_lights);
		CheckGLError();
	}
private:
	void _LoadData() {
		vector<shared_ptr<MeshLoadData>> new_meshes;
		vector<Light*> new_lights;
		_LoadMeshes(scene_path, new_meshes, new_lights);
		systems_manager->buffer->SetBaseMeshForInstancedCommand(new_meshes);
		_BufferMeshes(new_meshes);
		_AddRigidBodies(new_meshes);
		
		for (auto& mesh : new_meshes) {
			all_meshes.push_back(mesh->mesh_data);
		}
		for (auto& light : new_lights) {
			all_lights.push_back(light);
		}
		systems_manager->buffer->BufferTransform(all_meshes);
		
		_LoadAnimations();
	}
	void _LoadMeshes(string& path, vector<shared_ptr<MeshLoadData>>& out_new_meshes, vector<Light*>& out_new_lights) {
		bool is_load_armature = true;
		MeshManager::LoadMeshes(path, out_new_lights, out_new_meshes, is_load_armature);
		CheckGLError();
		_BlenderPostprocessLights(out_new_lights);
	}
	void _LoadAnimations() {
		for (auto& mesh : all_meshes) {
			for (auto& path : GetAnimationsPathsForModel(mesh->name)) {
				systems_manager->anim_manager->LoadAnimationForMesh(path.string(), mesh);
			}
		}
		CheckGLError();
	}

	void _LoadTransparentMeshes(string& path, vector<shared_ptr<MeshLoadData>>& out_new_meshes, vector<Light*>& out_new_lights) {
		MeshManager::LoadMeshes(path, out_new_lights, out_new_meshes);
		CheckGLError();
		_BlenderPostprocessLights(out_new_lights);
	}

	void _AddRigidBodies(vector<shared_ptr<MeshLoadData>>& new_meshes) {
		for (auto& mesh_ptr : new_meshes) {
			auto& mesh = mesh_ptr.get()->mesh_data;
			if (mesh->name.starts_with("Terrain")) {
				systems_manager->physics_manager->AddBoxRigidBody(mesh->min_AABB, mesh->max_AABB, 0.0f, mesh, mesh->transform);
			}
			else {
				systems_manager->physics_manager->AddBoxRigidBody(mesh->min_AABB, mesh->max_AABB, 10.0f, mesh, mesh->transform);
			}
		}
	}
	void _BufferMeshes(vector<shared_ptr<MeshLoadData>>& new_meshes) {
		for (auto& mesh_ptr : new_meshes) {
			if (mesh_ptr->mesh_data->base_mesh != nullptr) {
				continue;
			}
			auto mesh = mesh_ptr.get();
			mesh->mesh_data->texture = systems_manager->texture_manager->LoadTextureArray(mesh->tex_names);
			if (mesh->mesh_data->texture != nullptr) {
				mesh->mesh_data->texture->MakeResident();
			}
		}
		CheckGLError();
		systems_manager->buffer->BufferMeshData(new_meshes);
		CheckGLError();
	}
	void _ReloadScene() {
		_CleanupScene();
		_LoadData();
	}
	void _CleanupScene() {
		systems_manager->ResetBuffer();
		systems_manager->physics_manager->Reset();
		for (auto mesh : all_meshes) {
			delete mesh;
		}
		all_meshes.clear();
		for (auto mesh : all_transparent_meshes) {
			delete mesh;
		}
		all_transparent_meshes.clear();
		for (auto light : all_lights) {
			delete light;
		}
		all_lights.clear();
	}
	void _BlenderPostprocessLights(vector<Light*>& lights) {
		for (auto& light : lights)
		{
			light->constant_attenuation = 1;
			light->linear_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
			light->quadratic_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
			float denom = 10;
			light->color /= denom;
			light->specular /= denom;
		}
	}
	void _SortByDistance() {
		set<pair<float, MeshData*>, decltype([](const auto& lhs, const auto& rhs) {
			return lhs.first > rhs.first;
		}) > back_to_front;
		for (auto mesh : draw_meshes) {
			pair<float, MeshData*> distance_mesh = { distance(systems_manager->camera->pos, vec3(mesh->transform[3])) , mesh };
			back_to_front.insert(distance_mesh);
		}
		draw_meshes.clear();
		for (auto& mesh_dist : back_to_front) {
			draw_meshes.push_back(mesh_dist.second);
		}
	}
	void _LoadTerrain() {

	}
	void _OcclusionGather() {
		draw_lights = all_lights;
		draw_meshes = all_meshes;
	}
};