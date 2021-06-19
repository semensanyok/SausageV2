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

	Shader* blinn_phong;
	// custom draws per shader
	vector<MeshData*> all_meshes;
	vector<MeshData*> draw_meshes;
	vector<Light*> all_lights;
	vector<Light*> draw_lights;

	string scene_path = GetModelPath("frog.fbx");
	Scene(SystemsManager* systems_manager) :
		systems_manager{ systems_manager } {
	}
	~Scene() {};
	void Init() {
		_LoadData();
		blinn_phong = systems_manager->RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl");

		function<void()> scene_reload_callback = bind(&Scene::_ReloadScene, this);
		scene_reload_callback = bind(&Renderer::AddGlCommand, systems_manager->renderer, scene_reload_callback);
		systems_manager->file_watcher->AddCallback(scene_path, scene_reload_callback);

		CheckGLError();
	}
	void PrepareDraws() {
		_OcclusionGather();

		vector<DrawElementsIndirectCommand> commands;
		for (int i = 0; i < draw_meshes.size(); i++) {
			if (draw_meshes[i]->command.count != 0) {
				commands.push_back(draw_meshes[i]->command);
			}
		}
		systems_manager->buffer->BufferTransform(draw_meshes);
		systems_manager->buffer->AddCommands(commands);
		CheckGLError();
		systems_manager->buffer->BufferLights(draw_lights);
		CheckGLError();
		auto draw = new DrawCall{ systems_manager->buffer, blinn_phong, (unsigned int)commands.size(), 0, (int)draw_lights.size() };
		systems_manager->renderer->AddDraw(draw);
	}
private:
	void _LoadData() {
		_LoadMeshes();
	}
	void _LoadMeshes() {
		vector<shared_ptr<MeshLoadData>> new_meshes;
		vector<Light*> new_lights;

		MeshManager::LoadMeshes(scene_path, new_lights, new_meshes);
		CheckGLError();
		_BlenderPostprocessLights(new_lights);

		// optimization - merge same vertex count + texture as instanced draw.
		vector<shared_ptr<MeshLoadData>> instance_data;
		map<size_t, shared_ptr<MeshLoadData>> instanced_data_lookup;
		map<MeshData*, vector<MeshData*>> base_mesh_to_instances;
		for (auto& mesh_ptr : new_meshes) {
			auto mesh = mesh_ptr.get();
			all_meshes.push_back(mesh->mesh_data);
			auto key = mesh->tex_names.Hash() + mesh->vertices.size() + mesh->indices.size();
			auto base_mesh_ptr = instanced_data_lookup.find(key);
			if (base_mesh_ptr == instanced_data_lookup.end()) {
				instanced_data_lookup[key] = mesh_ptr;
				instance_data.push_back(mesh_ptr);
				continue;
			}
			auto base_mesh = (*base_mesh_ptr).second.get();
			auto& instance_count = base_mesh->instance_count;
			mesh->mesh_data->instance_id = instance_count++;
			base_mesh_to_instances[base_mesh->mesh_data].push_back(mesh->mesh_data);
		}
		for (auto& mesh_ptr : instance_data) {
			auto mesh = mesh_ptr.get();
			mesh->mesh_data->texture = systems_manager->texture_manager->LoadTextureArray(mesh->tex_names);
			if (mesh->mesh_data->texture != nullptr) {
				mesh->mesh_data->texture->MakeResident();
			}
		}
		CheckGLError();
		systems_manager->buffer->BufferMeshData(instance_data);
		for (auto& kv : base_mesh_to_instances) {
			auto& base = kv.first;
			for (auto& instance : kv.second) {
				instance->buffer_id = base->buffer_id;
				instance->transform_offset = base->transform_offset;
			}
		}
		CheckGLError();
		all_lights=new_lights;

		//for (auto& mesh : all_meshes) {
		//	mesh.buffer = systems_manager->buffer;
		//	if (mesh.name.starts_with("Terrain")) {
		//		vector<vector<Vertex>> v1 = { vertices[0] };
		//		vector<vector<unsigned int>> i1 = { indices[0] };
		//		vector<MeshData> m1 = { all_meshes[0] };
		//		vector<GLuint64> t1 = {tex_handles[mesh.id]};
		//		systems_manager->buffer->BufferMeshData(v1, i1, m1, t1, 0);
		//		//systems_manager->physics_manager->AddBoxRigidBody(mesh.min_AABB, mesh.max_AABB, 0.0f, &mesh, mesh.transform);
		//	}
		//	else {
		//		//systems_manager->physics_manager->AddBoxRigidBody(mesh.min_AABB, mesh.max_AABB, 10.0f, &mesh, mesh.transform);
		//	}
		//}
	}
	void _ReloadScene() {
		_CleanupScene();
		_LoadData();
		PrepareDraws();
	}
	void _CleanupScene() {
		systems_manager->ResetBuffer();
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