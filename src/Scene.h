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
			transforms[i] = draw_meshes[i].model;
		}
		systems_manager->buffer->BufferTransform(draw_meshes, transforms);
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
		new_lights.clear();
		new_lights.push_back(Light{ vec4(0,-1,0,0), vec4(0,7,0,0), vec4(50,50,50,0),vec4(50,50,50,0),LightType::Spot,  0.91, 0.82, 1, OGRE_P_L_ATT_DIST_7L, OGRE_P_L_ATT_DIST_7Q });
		new_lights.push_back(Light{ vec4(0,-1,0,0), vec4(5,5,5,0), vec4(50,50,50,0),vec4(50,50,50,0),LightType::Point,  0, 0, 1, OGRE_P_L_ATT_DIST_7L, OGRE_P_L_ATT_DIST_7Q });

		CheckGLError();
		for (auto mesh_id_tex : mesh_id_to_tex) {
			auto existing = diffuse_name_to_tex.find(mesh_id_tex.second.diffuse);
			if (existing == diffuse_name_to_tex.end()) {
				Texture* tex = systems_manager->texture_manager->LoadTextureArray(systems_manager->samplers.basic_repeat, mesh_id_tex.second);
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
	}
	void _LoadTerrain() {

	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
		draw_lights = all_lights;
	}
};