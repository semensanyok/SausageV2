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
	vector<MeshData> all_meshes;
	vector<MeshData> draw_meshes;
	vector<Light> all_lights;
	vector<Light> draw_lights;
	map<ShaderType, DrawCall*> draw_calls;
	
	string scene_path = GetModelPath("frog.fbx");
	Scene(SystemsManager* systems_manager) :
		systems_manager{ systems_manager } {
	}
	~Scene() {};
	void Init() {
		_LoadData();
		blinn_phong = systems_manager->RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl");
		
		function<void()> scene_reload_callback = bind(&Scene::ResetBuffer, this);
		scene_reload_callback = bind(&Renderer::AddGlCommand, systems_manager->renderer, scene_reload_callback);
		systems_manager->file_watcher->AddCallback(scene_path, scene_reload_callback);

		CheckGLError();
	}
	void PrepareDraws() {
		_OcclusionGather();
		
		vector<DrawElementsIndirectCommand> commands(draw_meshes.size());
		for (int i = 0; i < draw_meshes.size(); i++) {
			commands[i] = draw_meshes[i].command;
		}
		systems_manager->buffer->BufferTransform(draw_meshes);
		systems_manager->buffer->AddCommands(commands);
		CheckGLError();
		systems_manager->buffer->BufferLights(draw_lights);
		CheckGLError();
		auto draw = new DrawCall{ systems_manager->buffer, blinn_phong, (unsigned int)draw_meshes.size(), 0, (int)draw_lights.size() };
		systems_manager->renderer->AddDraw(draw);
	}
	void ResetBuffer() {
		all_meshes.clear();
		all_lights.clear();
		MeshManager::mesh_count = 0;
		systems_manager->ResetBuffer();
		_LoadData();
		PrepareDraws();
	}
private:
	void _LoadData() {
		_LoadMeshes();
	}
	void _LoadMeshes() {
		vector<MeshLoadData> new_meshes;
		vector<Light> new_lights;
		vector<DrawElementsIndirectCommand> new_commands;

		MeshManager::LoadMeshes(scene_path, new_lights, new_meshes);
		CheckGLError();
		_BlenderPostprocessLights(new_lights);

		// optimization - merge same vertex count + texture as instanced draw.
		//map<size_t, vector<MeshData>> instanced_data;
		//for (auto& mesh : new_meshes) {
		//	auto tex_names = mesh_id_to_tex[mesh.mesh_data.id];
		//	instanced_data[tex_names.Hash() + ].push_back(mesh);
		//}
		for (auto& mesh : new_meshes) {
			mesh.mesh_data.texture = systems_manager->texture_manager->LoadTextureArray(mesh.tex_names);
			if (mesh.mesh_data.texture != nullptr) {
				mesh.mesh_data.texture->MakeResident();
			}
		}
		CheckGLError();
		systems_manager->buffer->BufferMeshData(new_meshes);
		CheckGLError();
		all_lights=new_lights;

		all_meshes.clear();
		for (auto& mesh : new_meshes) {
			all_meshes.push_back(mesh.mesh_data);
		}

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
	void _BlenderPostprocessLights(vector<Light>& lights) {
		for (auto& light : lights)
		{
			light.constant_attenuation = 1;
			light.linear_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
			light.quadratic_attenuation = AttenuationConsts::OGRE_P_L_ATT_DIST_7L;
			float denom = 10;
			light.color /= denom;
			light.specular /= denom;
		}
	}
	void _LoadTerrain() {

	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
		draw_lights = all_lights;
	}
};