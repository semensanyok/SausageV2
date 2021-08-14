#pragma once

#include "../sausage.h"
#include "Renderer.h"
#include "Camera.h"
#include "Gui.h"
#include "../buffer.h"
#include "controller/Controller.h"
#include "TextureManager.h"
#include "../physics.h"
#include "AsyncTaskManager.h"
#include "Animation.h"
#include "StateManager.h"
#include "controller/MouseKeyboardEventProcessor.h"
#include "../Logging.h"
#include "../FileWatcher.h"

class SystemsManager
{
public:
	Camera* camera;
	Controller* controller;
	Renderer* renderer;
	TextureManager* texture_manager;
	FileWatcher* file_watcher;
	AsyncTaskManager* async_manager;
	AnimationManager* anim_manager;
	PhysicsManager* physics_manager;
	StateManager* state_manager;
	BulletDebugDrawer* bullet_debug_drawer;
	ControllerEventProcessor* controller_event_processor;

	BufferManager* buffer_manager;
	
	Shaders shaders;

	SystemsManager() {};
	~SystemsManager() {};
	
	void Render();
	void InitSystems();
	void ChangeStateUpdate();
	void Reset();

	void PreUpdate();
	void Update();
	void Clear();
	Shader* RegisterShader(const char* vs_name, const char* fs_name);
private:
	void _SubmitAsyncTasks();
	void _CreateDebugDrawer();
};