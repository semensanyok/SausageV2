#pragma once

#include "sausage.h"
#include "Renderer.h"
#include "RendererContextManager.h"
#include "Camera.h"
#include "Gui.h"
#include "Controller.h"
#include "TextureManager.h"
#include "FontManager.h"
#include "physics.h"
#include "AsyncTaskManager.h"
#include "Animation.h"
#include "StateManager.h"
#include "MouseKeyboardEventProcessor.h"
#include "Logging.h"
#include "FileWatcher.h"
#include "BufferManager.h"

class SystemsManager
{
public:
	MeshManager* mesh_manager;
	Camera* camera;
	Controller* controller;
	Renderer* renderer;
	RendererContextManager* renderer_context_manager;
	Samplers* samplers;
	TextureManager* texture_manager;
	FontManager* font_manager;
	FileWatcher* file_watcher;
	AsyncTaskManager* async_manager;
	AnimationManager* anim_manager;
	PhysicsManager* physics_manager;
	StateManager* state_manager;
	BulletDebugDrawer* bullet_debug_drawer;
	ControllerEventProcessorEditor* controller_event_processor;

	BufferManager* buffer_manager;
	
	Shaders* shaders;

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
	void _SetupShaders();
};
