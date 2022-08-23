#include "SystemsManager.h"
#include "Macros.h"

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
    shader_manager = new ShaderManager();
    shader_manager->SetupShaders();
	font_manager = new FontManager(samplers,mesh_manager, renderer, shader_manager->all_shaders);
	font_manager->Init();
	physics_manager = new PhysicsManager(state_manager);
  IF_SAUSAGE_DEBUG_DRAW_PHYSICS(
    _CreateDebugDrawer();
    bullet_debug_drawer->Activate();
    physics_manager->SetDebugDrawer(bullet_debug_drawer);
  );
	anim_manager = new AnimationManager(state_manager, mesh_manager, buffer_manager);

	async_manager = new AsyncTaskManager();
    screen_overlay_manager = new ScreenOverlayManager(buffer_manager->ui_buffer, shader_manager->all_shaders,mesh_manager,font_manager,renderer);
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
    bullet_debug_drawer = new BulletDebugDrawer(renderer, buffer_manager->bullet_debug_drawer_buffer,
      shader_manager->all_shaders, state_manager);
    //int debug_mask = btIDebugDraw::DBG_DrawWireframe | btIDebugDraw::DBG_DrawContactPoints;
    int debug_mask = btIDebugDraw::DBG_DrawWireframe;
    bullet_debug_drawer->setDebugMode(debug_mask);
  }
}
