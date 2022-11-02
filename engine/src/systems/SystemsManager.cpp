#include "SystemsManager.h"
#include "Macros.h"

void SystemsManager::InitSystems() {
	main_thread_id = this_thread::get_id();
	mesh_manager = new MeshManager();
	file_watcher = new FileWatcher();
	camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
    renderer_context_manager = new RendererContextManager();
	renderer_context_manager->InitContext();
	buffer_manager = new BufferManager(mesh_manager);
	buffer_manager->Init();
	renderer = new Renderer(renderer_context_manager, buffer_manager->storage);
    shader_manager = new ShaderManager(file_watcher, renderer, camera);
    shader_manager->SetupShaders();
    draw_call_manager = new DrawCallManager(shader_manager, renderer, buffer_manager->storage, mesh_manager);
    terrain_manager = new TerrainManager(buffer_manager, mesh_manager, draw_call_manager);
	state_manager = new StateManager(buffer_manager);
	samplers = new Samplers();
	samplers->Init();
	texture_manager = new TextureManager(samplers, buffer_manager);
	font_manager = new FontManager(samplers, texture_manager);
	font_manager->Init();
	physics_manager = new PhysicsManager(state_manager, buffer_manager->mesh_data_buffer);
  if (GameSettings::phys_debug_draw) {
    _CreateDebugDrawer();
    bullet_debug_drawer->Activate();
    physics_manager->SetDebugDrawer(bullet_debug_drawer);
  };
	anim_manager = new AnimationManager(state_manager, mesh_manager, buffer_manager);

	async_manager = new AsyncTaskManager();
    screen_overlay_manager = new ScreenOverlayManager(
      buffer_manager->ui_buffer,
      mesh_manager,
      font_manager,
      draw_call_manager);
    screen_overlay_manager->Init();
	controller_event_processor = new ControllerEventProcessorEditor(camera, screen_overlay_manager, buffer_manager);
    controller = new Controller(camera, state_manager, physics_manager);
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
	controller_event_processor->Update();
	camera->Update();
}

void SystemsManager::Clear() {
	renderer_context_manager->ClearContext();
	delete renderer;
	delete camera;
	delete async_manager;
  delete mesh_manager;
  delete file_watcher;
}

void SystemsManager::_SubmitAsyncTasks() {
	function<void()> log_io_task = bind(&Sausage::LogIO);
	function<void()> file_watcher_task = bind(&FileWatcher::Watch, file_watcher);
	function<void()> phys_sym_task = bind(&PhysicsManager::Simulate, physics_manager);
	function<void()> phys_update_task = bind(&PhysicsManager::UpdateTransforms, physics_manager);
	function<void()> play_anim = bind(&AnimationManager::PlayAnim, anim_manager);

	async_manager->SubmitMiscTask(log_io_task, true);
	async_manager->SubmitMiscTask(file_watcher_task, true);
	async_manager->SubmitPhysTask(phys_sym_task, true);
	async_manager->SubmitPhysTask(phys_update_task, true);
	async_manager->SubmitAnimTask(play_anim, true);
	function<void()> change_state_update = bind(&SystemsManager::ChangeStateUpdate, this);
	async_manager->SubmitMiscTask(change_state_update, true);
}

void SystemsManager::_CreateDebugDrawer() {
  if (bullet_debug_drawer == nullptr) {
    bullet_debug_drawer = new BulletDebugDrawer(buffer_manager, draw_call_manager, state_manager);
    //int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
    int debug_mask = btIDebugDraw::DBG_DrawWireframe;
    bullet_debug_drawer->setDebugMode(debug_mask);
  }
}
