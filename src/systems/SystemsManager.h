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
#include "systems/Animation.h"
#include "systems/StateManager.h"
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
	AnimationManager* anim_manager;
	PhysicsManager* physics_manager;
	StateManager* state_manager;
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
		state_manager = new StateManager();
		camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		renderer = new Renderer();
		renderer->InitContext();
		InitBuffer();

		shaders = {
			RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl"),
			RegisterShader("debug_vs.glsl", "debug_fs.glsl"),
			RegisterShader("stencil_vs.glsl", "stencil_fs.glsl")
		};

		physics_manager = new PhysicsManager(state_manager);
		controller = new Controller(camera, state_manager, physics_manager);
		anim_manager = new AnimationManager(state_manager);
		_CreateDebugDrawer();

		async_manager = new AsyncTaskManager();
		function<void()> log_io_task = bind(&Sausage::LogIO);
		function<void()> file_watcher_task = bind(&FileWatcher::Watch, file_watcher);
		function<void()> phys_sym_task = bind(&PhysicsManager::Simulate, physics_manager);
		function<void()> phys_update_task = bind(&PhysicsManager::UpdateTransforms, physics_manager);
		function<void()> play_anim = bind(&AnimationManager::PlayAnim, anim_manager);
		function<void()> buffer_mesh_update = bind(&StateManager::BufferUpdates, state_manager);

		async_manager->SubmitMiscTask(log_io_task, true);
		async_manager->SubmitMiscTask(file_watcher_task, true);
		async_manager->SubmitPhysTask(phys_sym_task, true);
		async_manager->SubmitPhysTask(phys_update_task, true);
		async_manager->SubmitPhysTask(buffer_mesh_update, true);
		async_manager->SubmitAnimTask(play_anim, true);

#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		physics_manager->SetDebugDrawer(bullet_debug_drawer);
#endif
		function<void()> change_state_update = bind(&SystemsManager::ChangeStateUpdate, this);
		async_manager->SubmitMiscTask(change_state_update, true);
	}
	void ChangeStateUpdate() {
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		static bool prev_phys_debug_draw = GameSettings::phys_debug_draw;
		if (prev_phys_debug_draw != GameSettings::phys_debug_draw) {
			if (GameSettings::phys_debug_draw) {
				bullet_debug_drawer->Activate();
				physics_manager->SetDebugDrawer(bullet_debug_drawer);
			}
			else {
				physics_manager->SetDebugDrawer(nullptr);
				bullet_debug_drawer->Deactivate();
			}
		}
		prev_phys_debug_draw = GameSettings::phys_debug_draw;
#endif
	}
	void _CreateDebugDrawer() {
		bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer, shaders.bullet_debug, state_manager);
		//int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
		int debug_mask = btIDebugDraw::DBG_DrawWireframe;
		bullet_debug_drawer->setDebugMode(debug_mask);
	}
	void Reset() {
		state_manager->Reset();
		physics_manager->Reset();
		buffer->Reset();
		MeshManager::Reset();
		anim_manager->Reset();
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