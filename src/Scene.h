#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "systems/Renderer.h"
#include "Structures.h"
#include "MeshManager.h"

class Scene {
	vector<MeshData> all_meshes;
	vector<DrawElementsIndirectCommand> all_commands;
	
	vector<MeshData> draw_meshes;
	vector<DrawElementsIndirectCommand> draw_commands;
	vector<Texture*> textures;

	BufferStorage* buffer;
	Camera* camera;
	Renderer* renderer;
	Samplers samplers;
public:
	Scene(BufferStorage* buffer, Camera* camera, Renderer* renderer, Samplers samplers) : camera{ camera }, buffer{ buffer }, renderer{ renderer }, samplers{ samplers } {
	}
	~Scene() {};
	void Init() {
		_LoadData();
	}
	void PrepareDraws() {
		draw_meshes = all_meshes;
		draw_commands = all_commands;
		UpdateMVP(camera->projection_view);
		buffer->SetCommands(draw_commands);
	}
	void UpdateMVP(mat4& projection_view) {
		vector<mat4> mvps(draw_meshes.size());
		for (int i = 0; i < draw_meshes.size(); i++) {
			mvps[i] = projection_view * draw_meshes[i].model;
		}
		buffer->BufferMvps(mvps, draw_meshes);
	}
private:
	void _LoadData() {
		vector<vector<Vertex>> vertices;
		vector<vector<unsigned int>> indices;
		vector<MeshData> new_meshes;
		vector<DrawElementsIndirectCommand> new_commands;
		vector<GLuint64> tex_handles;

		map<unsigned int, MaterialTexNames> mesh_id_to_tex;
		map<string, Texture*> diffuse_name_to_tex;
		
		MeshManager::LoadMeshes(vertices, indices, GetModelPath("cubes.fbx"), new_meshes, mesh_id_to_tex);
		CheckGLError();
		for (auto mesh_id_tex : mesh_id_to_tex) {
			auto existing = diffuse_name_to_tex.find(mesh_id_tex.second.diffuse_name);
			if (existing == diffuse_name_to_tex.end()) {
				Texture* tex = LoadTextureArray(samplers.basic_repeat, mesh_id_tex.second);
				if (tex != nullptr) {
					textures.push_back(tex);
					tex_handles.push_back(tex->texture_handle_ARB);
					diffuse_name_to_tex[mesh_id_tex.second.diffuse_name] = tex;
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
		new_commands = buffer->BufferMeshData(vertices, indices, new_meshes, tex_handles);

		CheckGLError();
		auto shader = renderer->RegisterShader("bindless_vs.glsl", "bindless_fs.glsl");
		CheckGLError();
		renderer->AddDraw(shader, buffer);
		CheckGLError();
		all_meshes.insert(all_meshes.end(), new_meshes.begin(), new_meshes.end());
		all_commands.insert(all_commands.end(), new_commands.begin(), new_commands.end());
	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
	}
};