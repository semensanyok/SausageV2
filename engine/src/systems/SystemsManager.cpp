#include "SystemsManager.h"
#include "Macros.h"
#include "Scene.h"

void SystemsManager::InitSystems() {
  main_thread_id = this_thread::get_id();

  file_watcher = new FileWatcher();
  camera = new Camera(60.0f, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT, 0.1f, 1000.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
  // NOTE: context must be initialized before any GL command (glGenBuffers, etc.)
  //       otherwise generates mysterious "access violation"
  renderer_context_manager = new RendererContextManager();
  renderer_context_manager->InitContext();

  command_buffers_manager = CommandBuffersManager::GetInstance();
  command_buffers_manager->InitBuffers();

  shader_manager = new ShaderManager(file_watcher, renderer, camera);
  shader_manager->SetupShaders();
  state_manager = new StateManager();
  draw_call_manager = new DrawCallManager(shader_manager, state_manager, command_buffers_manager);
  mesh_manager = new MeshManager(draw_call_manager);
  buffer_manager = new BufferManager(mesh_manager, texture_manager);
  buffer_manager->Init();
  spatial_manager = new SpatialManager(draw_call_manager, buffer_manager);
  renderer = new Renderer(renderer_context_manager, buffer_manager, spatial_manager, draw_call_manager, command_buffers_manager);
  samplers = new Samplers();
  samplers->Init();
  texture_manager = new TextureManager(samplers);
  font_manager = new FontManager(samplers, texture_manager);
  font_manager->Init();
  physics_manager = new PhysicsManager(state_manager, buffer_manager->mesh_data_buffer);
  mesh_data_utils = new MeshDataUtils(draw_call_manager, texture_manager, mesh_manager, buffer_manager, physics_manager);
  if (state_manager->phys_debug_draw) {
    CreateDebugDrawer();
    bullet_debug_drawer->Activate();
    physics_manager->SetDebugDrawer(bullet_debug_drawer);
  };
  anim_manager = new AnimationManager(state_manager, mesh_manager, buffer_manager);

  screen_overlay_manager = new ScreenOverlayManager(
    buffer_manager->ui_buffer,
    mesh_manager,
    font_manager,
    draw_call_manager);
  screen_overlay_manager->Init();

  controller_event_processor = new ControllerEventProcessorEditor(camera, screen_overlay_manager, buffer_manager);
  controller = new Controller(camera, state_manager, physics_manager);
  controller->AddProcessor(controller_event_processor);

  terrain_manager = new TerrainManager(buffer_manager, mesh_manager, draw_call_manager, physics_manager, texture_manager, spatial_manager);

  async_manager = new AsyncTaskManager();
  _SubmitAsyncTasks();

  AddDraws();
}

void SystemsManager::ChangeStateUpdate() {
  static bool prev_phys_debug_draw = state_manager->phys_debug_draw;
  if (prev_phys_debug_draw != state_manager->phys_debug_draw) {
    function<void()> f = bind(&SystemsManager::_ChangePhysicsDebugDrawer, this);
    renderer->AddGlCommand(f, false);
  }
  prev_phys_debug_draw = state_manager->phys_debug_draw;
}

void SystemsManager::_ChangePhysicsDebugDrawer() {
  if (state_manager->phys_debug_draw) {
    CreateDebugDrawer();
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

void SystemsManager::AddDraws()
{
  renderer->AddDraw(draw_call_manager->font_ui_dc, DrawOrder::UI_TEXT);
  //renderer->AddDraw(overlay_3d_dc, DrawOrder::OVERLAY_3D);
  renderer->AddDraw(draw_call_manager->back_ui_dc, DrawOrder::UI_BACK);
  renderer->AddDraw(draw_call_manager->mesh_dc, DrawOrder::MESH);
  renderer->AddDraw(draw_call_manager->terrain_dc, DrawOrder::TERRAIN);
  renderer->AddDraw(draw_call_manager->mesh_static_dc, DrawOrder::MESH_STATIC);
  renderer->AddDraw(draw_call_manager->outline_dc, DrawOrder::OUTLINE);
}

void SystemsManager::CreateDebugDrawer() {
  if (bullet_debug_drawer == nullptr) {
    bullet_debug_drawer = new BulletDebugDrawer(buffer_manager, draw_call_manager, state_manager);
  }
}
