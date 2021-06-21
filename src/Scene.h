#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "systems/Renderer.h"
#include "Structures.h"
#include "systems/MeshManager.h"
#include "systems/TextureManager.h"
#include "FileWatcher.h"
#include "systems/SystemsManager.h"

class Scene {
public:
	SystemsManager* systems_manager;

	DrawCall* draw_call;
	const unsigned int command_buffer_size;
	Shader* blinn_phong;
	// custom draws per shader
	vector<MeshData*> all_meshes;
	vector<MeshData*> draw_meshes;
	vector<Light*> all_lights;
	vector<Light*> draw_lights;

	string scene_path = GetModelPath("frog.fbx");
	Scene(SystemsManager* systems_manager) :
		systems_manager{ systems_manager }, blinn_phong{ systems_manager->blinn_phong }, command_buffer_size{ systems_manager->buffer->MAX_COMMAND }{
		draw_call = new DrawCall();
		draw_call->shader = blinn_phong;
		draw_call->mode = GL_TRIANGLES;
		draw_call->buffer = systems_manager->buffer;
		draw_call->command_buffer = draw_call->buffer->CreateCommandBuffer(command_buffer_size);
		
		systems_manager->renderer->AddDraw(draw_call);
	}
	~Scene() {};
	void Init() {
		_LoadData();

		function<void()> scene_reload_callback = bind(&Scene::_ReloadScene, this);
		bool is_persistent_command = false;
		scene_reload_callback = bind(&Renderer::AddGlCommand, systems_manager->renderer, scene_reload_callback, is_persistent_command);
		systems_manager->file_watcher->AddCallback(scene_path, scene_reload_callback);

		CheckGLError();
	}
	void PrepareDraws() {
		_OcclusionGather();

		vector<DrawElementsIndirectCommand> commands;
		for (int i = 0; i < draw_meshes.size(); i++) {
			if (draw_meshes[i]->base_mesh == nullptr) {
				commands.push_back(draw_meshes[i]->command);
			}
		}
		systems_manager->buffer->BufferTransform(draw_meshes);
		systems_manager->buffer->AddCommands(commands, draw_call->command_buffer, command_buffer_size);
		CheckGLError();
		systems_manager->buffer->BufferLights(draw_lights);
		CheckGLError();
		draw_call->command_count = (unsigned int)commands.size();
		draw_call->num_lights = (int)draw_lights.size();
	}
private:
	void _LoadData() {
		vector<shared_ptr<MeshLoadData>> new_meshes;
		vector<Light*> new_lights;
		_LoadMeshes(new_meshes, new_lights);
		_SetBaseMeshForInstancedCommand(new_meshes);
		_BufferMeshes(new_meshes);
		_AddRigidBodies(new_meshes);
		
		for (auto& mesh : new_meshes) {
			all_meshes.push_back(mesh->mesh_data);
		}
		all_lights=new_lights;
	}
	void _LoadMeshes(vector<shared_ptr<MeshLoadData>>& out_new_meshes, vector<Light*>& out_new_lights) {
		MeshManager::LoadMeshes(scene_path, out_new_lights, out_new_meshes);
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
		// optimization - merge same vertex count + texture as instanced draw.
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
	void _SetBaseMeshForInstancedCommand(vector<shared_ptr<MeshLoadData>>& new_meshes) {
		map<size_t, shared_ptr<MeshLoadData>> instanced_data_lookup;
		for (auto& mesh_ptr : new_meshes) {
			auto mesh = mesh_ptr.get();
			auto key = mesh->tex_names.Hash() + mesh->vertices.size() + mesh->indices.size();
			auto base_mesh_ptr = instanced_data_lookup.find(key);
			if (base_mesh_ptr == instanced_data_lookup.end()) {
				instanced_data_lookup[key] = mesh_ptr;
				continue;
			}
			auto base_mesh = (*base_mesh_ptr).second.get();
			auto& instance_count = base_mesh->instance_count;
			mesh->mesh_data->instance_id = instance_count++;
			mesh->mesh_data->base_mesh = base_mesh->mesh_data;
		}
	}
	void _ReloadScene() {
		_CleanupScene();
		_LoadData();
		PrepareDraws();
	}
	void _CleanupScene() {
		systems_manager->ResetBuffer();
		systems_manager->physics_manager->Reset();
		for (auto mesh : all_meshes) {
			delete mesh;
		}
		all_meshes.clear();
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
	void _LoadTerrain() {

	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
		draw_lights = all_lights;
	}
};