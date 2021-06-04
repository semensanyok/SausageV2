#pragma once

#include "sausage.h"
#include "systems/BufferStorage.h"
#include "systems/Renderer.h"
#include "Structures.h"

class Scene {
	vector<MeshData> all_meshes;
	vector<DrawElementsIndirectCommand> all_commands;
	
	vector<MeshData> draw_meshes;
	vector<DrawElementsIndirectCommand> draw_commands;

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

		LoadMeshes(vertices, indices, all_meshes, GetModelPath("cubes.fbx"));
		Texture* tex = LoadTextureArray("Image0000.png", "Image0000_normal.png", "Image0000_specular.png", "Image0000_height.png", samplers.basic_repeat);
		tex->MakeResident();
		vector<GLuint64> texture_ids(all_meshes.size(), tex->texture_handle_ARB);
		CheckGLError();
		all_commands = buffer->BufferMeshData(vertices, indices, all_meshes, texture_ids);
		CheckGLError();
		auto shader = renderer->RegisterShader("bindless_vs.glsl", "bindless_fs.glsl");
		CheckGLError();
		renderer->AddDraw(shader, buffer);
		CheckGLError();
	}
	void _OcclusionGather() {
		draw_meshes = all_meshes;
	}
};