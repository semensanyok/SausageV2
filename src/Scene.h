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
		
		function<void()> scene_reload_callback = bind(&Scene::ReloadBuffer, this);
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
		UpdateMVP();
		systems_manager->buffer->AddCommands(commands);
		CheckGLError();
		systems_manager->buffer->BufferLights(draw_lights);
		CheckGLError();
		auto draw = new DrawCall{ systems_manager->buffer, blinn_phong, (unsigned int)draw_meshes.size(), 0, (int)draw_lights.size() };
		systems_manager->renderer->AddDraw(draw);
	}
	void UpdateMVP() {
		vector<mat4> transforms(draw_meshes.size());
		for (int i = 0; i < draw_meshes.size(); i++) {
			transforms[i] = draw_meshes[i].transform;
		}
		systems_manager->buffer->BufferTransform(draw_meshes);
	}
	void ReloadBuffer() {
		all_meshes.clear();
		all_lights.clear();
		MeshManager::mesh_count = 0;
		systems_manager->ReloadBuffer();
		_LoadData();
		PrepareDraws();
	}
private:
	void _LoadData() {
		_LoadMeshes();
	}
	void _LoadMeshes() {
		vector<vector<Vertex>> vertices;
		vector<vector<unsigned int>> indices;
		vector<MeshData> new_meshes;
		vector<Light> new_lights;
		vector<DrawElementsIndirectCommand> new_commands;
		vector<GLuint64> tex_handles;

		map<unsigned int, MaterialTexNames> mesh_id_to_tex;
		map<string, Texture*> diffuse_name_to_tex;

		MeshManager::LoadMeshes(vertices, indices, scene_path, new_meshes, new_lights, mesh_id_to_tex);
		_BlenderPostprocessLights(new_lights);

		CheckGLError();
		for (auto& mesh_id_tex : mesh_id_to_tex) {
			auto existing = diffuse_name_to_tex.find(mesh_id_tex.second.diffuse);
			if (existing == diffuse_name_to_tex.end()) {
				Texture* tex = systems_manager->texture_manager->LoadTextureArray(mesh_id_tex.second);
				if (tex != nullptr) {
					tex_handles.push_back(tex->texture_handle_ARB);
					diffuse_name_to_tex[mesh_id_tex.second.diffuse] = tex;
					tex->MakeResident();
				}
				else {
					tex_handles.push_back(0);
				}
			}
			else {
				tex_handles.push_back(existing->second->texture_handle_ARB);
			}
		}
		CheckGLError();
		systems_manager->buffer->BufferMeshData(vertices, indices, new_meshes, tex_handles);
		CheckGLError();
		all_meshes=new_meshes;
		all_lights=new_lights;

		for (auto& mesh : all_meshes) {
			mesh.buffer = systems_manager->buffer;
			if (mesh.name.starts_with("Terrain")) {
				vector<vector<Vertex>> v1 = { vertices[0] };
				vector<vector<unsigned int>> i1 = { indices[0] };
				vector<MeshData> m1 = { all_meshes[0] };
				vector<GLuint64> t1 = {tex_handles[mesh.id]};
				systems_manager->buffer->BufferMeshData(v1, i1, m1, t1, 0);
				//systems_manager->physics_manager->AddBoxRigidBody(mesh.min_AABB, mesh.max_AABB, 0.0f, &mesh, mesh.transform);
			}
			else {
				//systems_manager->physics_manager->AddBoxRigidBody(mesh.min_AABB, mesh.max_AABB, 10.0f, &mesh, mesh.transform);
			}
		}
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