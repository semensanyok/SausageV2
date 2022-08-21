#include "SystemsManager.h"
#include "Macros.h"

void SystemsManager::Render() {
	renderer->Render(camera);
}

void SystemsManager::InitSystems() {
	main_thread_id = this_thread::get_id();
	mesh_manager = new MeshManager();
  SausageSystems::registry["MeshManager"] = mesh_manager;
	file_watcher = new FileWatcher();
  SausageSystems::registry["FileWatcher"] = file_watcher;
	camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
  SausageSystems::registry["Camera"] = camera;
    renderer_context_manager = new RendererContextManager();
  SausageSystems::registry["RendererContextManager"] = renderer_context_manager;
	renderer = new Renderer(renderer_context_manager);
  SausageSystems::registry["Renderer"] = renderer;
	renderer_context_manager->InitContext();
	buffer_manager = new BufferManager(mesh_manager);
	buffer_manager->Init();
  SausageSystems::registry["BufferManager"] = buffer_manager;
	state_manager = new StateManager(buffer_manager);
  SausageSystems::registry["StateManager"] = state_manager;
	samplers = new Samplers();
	samplers->Init();
  SausageSystems::registry["Samplers"] = samplers;
	texture_manager = new TextureManager(samplers);
  SausageSystems::registry["TextureManager"] = texture_manager;
	_SetupShaders();
	font_manager = new FontManager(samplers,mesh_manager, renderer, shaders);
  SausageSystems::registry["FontManager"] = font_manager;
	font_manager->Init();
	physics_manager = new PhysicsManager(state_manager);
  SausageSystems::registry["PhysicsManager"] = physics_manager;
  IF_SAUSAGE_DEBUG_DRAW_PHYSICS(
    _CreateDebugDrawer();
    bullet_debug_drawer->Activate();
    physics_manager->SetDebugDrawer(bullet_debug_drawer);
  );
	anim_manager = new AnimationManager(state_manager, mesh_manager, buffer_manager);
  SausageSystems::registry["AnimationManager"] = anim_manager;

	async_manager = new AsyncTaskManager();
  SausageSystems::registry["AsyncTaskManager"] = async_manager;
    screen_overlay_manager = new ScreenOverlayManager(buffer_manager->ui_buffer,shaders,mesh_manager,font_manager,renderer);
    screen_overlay_manager->Init();
  SausageSystems::registry["ScreenOverlayManager"] = screen_overlay_manager;
	controller_event_processor = new ControllerEventProcessorEditor(camera, screen_overlay_manager, buffer_manager);
  SausageSystems::registry["ControllerEventProcessorEditor"] = controller_event_processor;
    controller = new Controller(camera, state_manager, physics_manager);
  SausageSystems::registry["Controller"] = controller;
    controller->AddProcessor(controller_event_processor);
	_SubmitAsyncTasks();
}

void SystemsManager::ChangeStateUpdate() {
  static bool prev_phys_debug_draw = GameSettings::phys_debug_draw;
  if (prev_phys_debug_draw != GameSettings::phys_debug_draw) {
    function<void()> f = bind(&SystemsManager::_ChangePhysicsDebugDrawer, this);
    renderer->AddGlCommand(f, false);
  }
  prev_phys_debug_draw = GameSettings::phys_debug_draw;
}

void SystemsManager::_ChangePhysicsDebugDrawer() {
  if (GameSettings::phys_debug_draw) {
    _CreateDebugDrawer();
    bullet_debug_drawer->Activate();
    physics_manager->SetDebugDrawer(bullet_debug_drawer);
  }
  else {
    physics_manager->SetDebugDrawer(nullptr);
    bullet_debug_drawer->Deactivate();
  }
}

void SystemsManager::Reset() {
	state_manager->Reset();
	physics_manager->Reset();
	buffer_manager->Reset();
	mesh_manager->Reset();
	anim_manager->Reset();
}

void SystemsManager::PreUpdate() {
	state_manager->UpdateDeltaTimeTimings();
	camera->PreUpdate(state_manager->delta_time);
	font_manager->PreUpdate();
}

void SystemsManager::Update() {
	controller->Update();
	camera->Update();
	controller_event_processor->Update();
}

void SystemsManager::Clear() {
	renderer_context_manager->ClearContext();
	delete renderer;
	delete camera;
	delete async_manager;
  delete mesh_manager;
  delete file_watcher;
}

Shader* SystemsManager::RegisterShader(const char* vs_name, const char* fs_name) {
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

void SystemsManager::_SubmitAsyncTasks() {
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
	function<void()> change_state_update = bind(&SystemsManager::ChangeStateUpdate, this);
	async_manager->SubmitMiscTask(change_state_update, true);
}

void SystemsManager::_CreateDebugDrawer() {
  if (bullet_debug_drawer == nullptr) {
    bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer_manager->bullet_debug_drawer_buffer, shaders, state_manager);
    //int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
    int debug_mask = btIDebugDraw::DBG_DrawWireframe;
    bullet_debug_drawer->setDebugMode(debug_mask);
  }
}

void SystemsManager::_SetupShaders() {
	shaders = new Shaders{
		RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl"),
		RegisterShader("debug_vs.glsl", "debug_fs.glsl"),
		RegisterShader("stencil_vs.glsl", "stencil_fs.glsl"),
		RegisterShader("ui_font_vs.glsl", "ui_font_fs.glsl"),
    RegisterShader("ui_back_vs.glsl", "ui_back_fs.glsl"),
    RegisterShader("3d_font_vs.glsl", "3d_font_fs.glsl"),
	};
	shaders->blinn_phong->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
	shaders->blinn_phong->SetVec3Uniform(string("view_pos"), &(camera->pos));

  shaders->bullet_debug->SetMat4Uniform(string("projection_view"), &(camera->projection_view));

  shaders->font_ui->SetMat4Uniform(string("projection_ortho"), &(camera->projection_matrix_ortho));

  shaders->back_ui->SetMat4Uniform(string("projection_ortho"), &(camera->projection_matrix_ortho));

  shaders->font_3d->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
}

