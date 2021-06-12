#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "systems/Renderer.h"
#include "Structures.h"
#include "systems/MeshManager.h"
#include "systems/TextureManager.h"
#include "FileWatcher.h"

class Scene {
public:
	BufferStorage* buffer;
	TextureManager* texture_manager;
	Camera* camera;
	Renderer* renderer;
	FileWatcher* file_watcher;
	Samplers samplers;

	Shader* blinn_phong;
	// custom draws per shader
	vector<MeshData> all_meshes;
	vector<MeshData> draw_meshes;
	vector<Light> all_lights;
	vector<Light> draw_lights;
	map<ShaderType, DrawCall*> draw_calls;
	
	Scene(BufferStorage* buffer, TextureManager* texture_manager, Camera* camera, Renderer* renderer, FileWatcher* file_watcher, Samplers samplers) : 
		camera{ camera }, 
		buffer{ buffer }, 
		texture_manager{ texture_manager }, 
		renderer{ renderer }, 
		file_watcher{ file_watcher }, 
		samplers{ samplers } {

	}
	~Scene() {};\
	void Init() {
		_LoadData();
		blinn_phong = renderer->RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl", file_watcher);
		function<void()> gl_command = bind(&Shader::InitOrReload, blinn_phong);
		function<void()> callback = bind(&Renderer::AddGlCommand, renderer, gl_command);
		file_watcher->AddCallback(blinn_phong->fragment_path, callback);

		CheckGLError();
	}
	void PrepareDraws() {
		_OcclusionGather();
		auto draw = new DrawCall{ buffer, blinn_phong, (unsigned int)draw_meshes.size(), 0 };
		renderer->AddDraw(draw);
		vector<DrawElementsIndirectCommand> commands(draw_meshes.size());
		for (int i = 0; i < draw_meshes.size(); i++) {
			commands[i] = draw_meshes[i].command;
		}
		UpdateMVP();
		buffer->AddCommands(commands);
		CheckGLError();
		draw_lights[0].position = vec3(-2.7, 1.7, 8.4);
		draw_lights[1].position = vec3(0, -5, 5);

		buffer->BufferLights(draw_lights);
		CheckGLError();
	}
	void UpdateMVP() {
		vector<mat4> transforms(draw_meshes.size());
		for (int i = 0; i < draw_meshes.size(); i++) {
			transforms[i] = draw_meshes[i].model;
		}
		buffer->BufferTransform(draw_meshes, transforms);
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

		//MeshManager::LoadMeshes(vertices, indices, GetModelPath("cubes.fbx"), new_meshes, new_lights, mesh_id_to_tex);
		MeshManager::LoadMeshes(vertices, indices, GetModelPath("cubes.fbx"), new_meshes, new_lights, mesh_id_to_tex);
		CheckGLError();
		for (auto mesh_id_tex : mesh_id_to_tex) {
			auto existing = diffuse_name_to_tex.find(mesh_id_tex.second.diffuse);
			if (existing == diffuse_name_to_tex.end()) {
				Texture* tex = texture_manager->LoadTextureArray(samplers.basic_repeat, mesh_id_tex.second);
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
		buffer->BufferMeshData(vertices, indices, new_meshes, tex_handles);
		CheckGLError();
		all_meshes.insert(all_meshes.end(), new_meshes.begin(), new_meshes.end());
		all_lights.insert(all_lights.end(), new_lights.begin(), new_lights.end());
	}
	void _LoadTerrain() {

	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
		draw_lights = all_lights;
	}
};