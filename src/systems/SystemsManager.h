#pragma once

#include "sausage.h"
#include "systems/Renderer.h"
#include "systems/Camera.h"
#include "systems/Gui.h"
#include "systems/BufferStorage.h"
#include "systems/Controller.h"
#include "TextureManager.h"
#include "systems/Physics.h"
#include "systems/BulletDebugDrawer.h"

class SystemsManager {
public:
	Camera* camera;
	BufferStorage* buffer;
	Renderer* renderer;
	TextureManager* texture_manager;
	FileWatcher* file_watcher;
	
	PhysicsManager* physics_manager;
	BulletDebugDrawer* bullet_debug_drawer;
	
	Shader* blinn_phong;
	Shader* bullet_debug;

	float delta_time = 0;
	float last_ticks = 0;

	SystemsManager() {};
	~SystemsManager() {};
	
	void Render() {
		renderer->Render(camera);
	}

	void InitSystems() {
		texture_manager = new TextureManager();
		file_watcher = new FileWatcher();
		//camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		renderer = new Renderer();
		renderer->InitContext();
		InitBuffer();

		blinn_phong = RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl");
		bullet_debug = RegisterShader("debug_vs.glsl", "debug_fs.glsl");
		bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer, bullet_debug);
		bullet_debug_drawer->setDebugMode(btIDebugDraw::DBG_DrawWireframe);
		physics_manager = new PhysicsManager(bullet_debug_drawer);
	}

	void ResetBuffer() {
		buffer->Reset();
	}

	Shader* RegisterShader(const char* vs_name, const char* fs_name) {
		auto shader = renderer->RegisterShader(vs_name, fs_name);
		bool is_persistent_command = false;
		function<void()> fs_reload_callback = bind(&Shader::ReloadFS, shader);
		fs_reload_callback = bind(&Renderer::AddGlCommand, renderer, fs_reload_callback, is_persistent_command);
		file_watcher->AddCallback(shader->fragment_path, fs_reload_callback);
		
		function<void()> vs_reload_callback = bind(&Shader::ReloadVS, shader);
		vs_reload_callback = bind(&Renderer::AddGlCommand, renderer, vs_reload_callback, is_persistent_command);
		file_watcher->AddCallback(shader->vertex_path, vs_reload_callback);

		return shader;
	}

	void Clear() {
		DisposeBuffer();
		renderer->ClearContext();
		delete renderer;
		delete camera;
	}
	void UpdateDeltaTime() {
		float this_ticks = SDL_GetTicks();
		delta_time = this_ticks - last_ticks;
		last_ticks = this_ticks;
	}
private:
	void InitBuffer() {
		buffer = new BufferStorage();
		buffer->InitMeshBuffers();
		CheckGLError();
	}
	void DisposeBuffer() {
		renderer->RemoveBuffer(buffer);
		buffer->Dispose();
		delete buffer;
	}
};