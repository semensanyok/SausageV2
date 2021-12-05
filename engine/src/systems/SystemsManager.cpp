#include "SystemsManager.h"

void SystemsManager::Render() {
	renderer->Render(camera);
}

void SystemsManager::InitSystems() {
	main_thread_id = this_thread::get_id();
	mesh_manager = new MeshManager();
	file_watcher = new FileWatcher();
	camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
	renderer_context_manager = new RendererContextManager();
	renderer = new Renderer(renderer_context_manager);
	renderer_context_manager->InitContext();
	buffer_manager = new BufferManager(mesh_manager);
	buffer_manager->Init();
	state_manager = new StateManager(buffer_manager);
	samplers = new Samplers();
	samplers->Init();
	texture_manager = new TextureManager(samplers);
	_SetupShaders();
	font_manager = new FontManager(samplers,
    buffer_manager->overlay_3d_buffer,
    mesh_manager, renderer, shaders);
	font_manager->Init();
	physics_manager = new PhysicsManager(state_manager);
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
	_CreateDebugDrawer();
	bullet_debug_drawer->Activate();
	physics_manager->SetDebugDrawer(bullet_debug_drawer);
#endif
	controller_event_processor = new ControllerEventProcessorEditor(camera);
	controller = new Controller(camera, state_manager, physics_manager, controller_event_processor);
	anim_manager = new AnimationManager(state_manager, mesh_manager, buffer_manager);

	async_manager = new AsyncTaskManager();
  screen_overlay_manager = new ScreenOverlayManager(buffer_manager->ui_buffer,shaders,mesh_manager,font_manager);
	_SubmitAsyncTasks();
}

void SystemsManager::ChangeStateUpdate() {
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
	bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer_manager->bullet_debug_drawer_buffer, shaders, state_manager);
	//int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
	int debug_mask = btIDebugDraw::DBG_DrawWireframe;
	bullet_debug_drawer->setDebugMode(debug_mask);
}

void SystemsManager::_SetupShaders() {
	shaders = new Shaders{
		RegisterShader("blinn_phong_vs.glsl", "blinn_phong_fs.glsl"),
		RegisterShader("debug_vs.glsl", "debug_fs.glsl"),
		RegisterShader("stencil_vs.glsl", "stencil_fs.glsl"),
		RegisterShader("ui_font_vs.glsl", "ui_font_fs.glsl"),
    RegisterShader("3d_font_vs.glsl", "3d_font_fs.glsl"),
	};
	shaders->blinn_phong->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
	shaders->blinn_phong->SetVec3Uniform(string("view_pos"), &(camera->pos));
	shaders->bullet_debug->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
	shaders->font_ui->SetMat4Uniform(string("projection_ortho"), &(camera->projection_matrix_ortho));
  shaders->font_3d->SetMat4Uniform(string("projection_view"), &(camera->projection_view));
}

