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
#include "systems/AsyncTaskManager.h"
#include "Logging.h"
#include "FileWatcher.h"

class SystemsManager {
public:
	Camera* camera;
	Controller* controller;
	BufferStorage* buffer;
	Renderer* renderer;
	TextureManager* texture_manager;
	FileWatcher* file_watcher;
	AsyncTaskManager* async_manager;

	PhysicsManager* physics_manager;
	BulletDebugDrawer* bullet_debug_drawer = nullptr;
	
	Shaders shaders;

	SystemsManager() {};
	~SystemsManager() {};
	
	void Render() {
		renderer->Render(camera);
	}

	void InitSystems() {
		texture_manager = new TextureManager();
		file_watcher = new FileWatcher();
		//camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		renderer = new Renderer();
		renderer->InitContext();
		InitBuffer();

		shaders = {
			RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl"),
			RegisterShader("debug_vs.glsl", "debug_fs.glsl"),
			RegisterShader("stencil_vs.glsl", "stencil_fs.glsl")
		};

#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer, shaders.bullet_debug);
		//int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
		int debug_mask = btIDebugDraw::DBG_DrawWireframe;
		bullet_debug_drawer->setDebugMode(debug_mask);
#endif
		physics_manager = new PhysicsManager(bullet_debug_drawer);
		controller = new Controller(camera, physics_manager);

		async_manager = new AsyncTaskManager();
		function<void()> log_io_task = bind(&Sausage::LogIO);
		function<void()> file_watcher_task = bind(&FileWatcher::Watch, file_watcher);
		function<void()> phys_sym_task = bind(&PhysicsManager::Simulate, physics_manager);
		function<void()> phys_update_task = bind(&PhysicsManager::UpdateTransforms, physics_manager);
		async_manager->SubmitMiscTask(log_io_task, true);
		async_manager->SubmitMiscTask(file_watcher_task, true);
		async_manager->SubmitPhysTask(phys_sym_task, true);
		async_manager->SubmitPhysTask(phys_update_task, true);
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
		delete async_manager;
	}
	void UpdateDeltaTime() {
		float this_ticks = SDL_GetTicks();
		GameSettings::delta_time = this_ticks - GameSettings::last_ticks;
		GameSettings::last_ticks = this_ticks;
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