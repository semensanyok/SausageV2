#include "Renderer.h"

using namespace std;

void Renderer::Render(Camera *camera) {
#ifdef SAUSAGE_PROFILE_ENABLE
  auto proft1 = chrono::steady_clock::now();
#endif
  _ExecuteCommands();
#ifdef SAUSAGE_PROFILE_ENABLE
  auto proft2 = chrono::steady_clock::now();
#endif
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  // glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
  // glStencilFunc(GL_NOT, 1, 0xFF);
  // glStencilMask(0xFF);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  {
    for (auto buffer_shader : buffer_to_draw_call) {
      if (buffer_shader.second.empty()) {
        continue;
      }
      auto buffer = (*buffer_shader.second.begin())->buffer;
      {
        buffer->PreDraw();
        for (auto draw : buffer_shader.second) {
          if (draw->command_count > 0) {
            glUseProgram(draw->shader->id);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw->command_buffer->id);
            draw->shader->setMat4("projection_view", camera->projection_view);
            draw->shader->setVec3("view_pos", camera->pos);
            glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT, nullptr,
                                        draw->command_count, 0);
          }
        }
        buffer->PostDraw();
      }
      // CheckGLError();
    }
#ifdef SAUSAGE_PROFILE_ENABLE
    auto proft3 = chrono::steady_clock::now();
#endif
    Gui::RenderGui(window, camera);
#ifdef SAUSAGE_PROFILE_ENABLE
    auto proft4 = chrono::steady_clock::now();
#endif
    SDL_GL_SwapWindow(window);
#ifdef SAUSAGE_PROFILE_ENABLE
    auto proft5 = chrono::steady_clock::now();
    ProfTime::render_total_ns = proft5 - proft1;
    ProfTime::render_commands_ns = proft2 - proft1;
    ProfTime::render_draw_ns = proft3 - proft2;
    ProfTime::render_gui_ns = proft4 - proft3;
    ProfTime::render_swap_window_ns = proft5 - proft4;
#endif
  }
  Events::end_render_frame_event.notify_all();
}

void Renderer::RemoveBuffer(BufferStorage *buffer) {
  auto buf = buffer_to_draw_call.find(buffer->id);
  if (buf != buffer_to_draw_call.end()) {
    for (auto draw : (*buf).second) {
      buf_shad_ids.erase(pair(draw->buffer->GetBufferId(), draw->shader->id));
      delete draw;
    }
    buffer_to_draw_call.erase(buf);
  }
}

void Renderer::AddGlCommand(function<void()> &f, bool is_persistent) {
  gl_commands.Push(pair(f, is_persistent));
}

Shader *Renderer::RegisterShader(const char *vs_name, const char *fs_name) {
  for (auto shader : shaders) {
    if (shader.second->vertex_path.ends_with(vs_name) ||
        shader.second->fragment_path.ends_with(fs_name)) {
      LOG((ostringstream() << "Shader with vs_name=" << string(vs_name)
                           << " fs_name=" << string(fs_name)
                           << " already registered")
              .str());
      return shader.second;
    }
  }
  Shader *shader = new Shader(GetShaderPath(vs_name), GetShaderPath(fs_name));
  shader->InitOrReload();
  shaders[shader->id] = shader;
  return shader;
}

bool Renderer::AddDraw(DrawCall *draw) {
  if (!shaders.contains(draw->shader->id)) {
    LOG((ostringstream() << "Unable to add draw for unregistered shader: "
                         << draw->shader->id
                         << " vs:" << draw->shader->vertex_path
                         << " fs:" << draw->shader->fragment_path)
            .str());
    return false;
  }
  pair<int, int> buf_shad_id{draw->buffer->GetBufferId(), draw->shader->id};
  // if (buf_shad_ids.contains(buf_shad_id)) {
  //	LOG((ostringstream() << "Draw for shader: " << draw->shader->id << "
  //buffer:" << draw->buffer->id << "already exists").str()); 	return false;
  //}
  buffer_to_draw_call[draw->buffer->GetBufferId()].push_back(draw);
  buf_shad_ids.insert(buf_shad_id);
  return true;
}

bool Renderer::RemoveDraw(DrawCall *draw) {
  auto btd_ptr = buffer_to_draw_call.find(draw->buffer->GetBufferId());
  if (btd_ptr == buffer_to_draw_call.end()) {
    LOG("Unable to remove draw, buffer not found");
    return false;
  }
  auto &draws = btd_ptr->second;
  auto cur_draw = draws.begin();
  while (!(cur_draw == draws.end())) {
    if (*cur_draw == draw) {
      draws.erase(cur_draw);
      return true;
    }
    cur_draw++;
  }
  return false;
}

void Renderer::InitContext() {
  SDL_Init(SDL_INIT_VIDEO);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

  // SDL_ShowCursor(SDL_ENABLE);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef GL_DEBUG
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

  SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

  window = SDL_CreateWindow("caveview", SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED, GameSettings::SCR_WIDTH,
                            GameSettings::SCR_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE |
                                SDL_WINDOW_ALLOW_HIGHDPI // SDL_WINDOW_VULKAN
  );
  if (!window)
    LOG("Couldn't create window");

  context = SDL_GL_CreateContext(window);
  if (!context)
    LOG("Couldn't create context");

  SDL_GL_MakeCurrent(window, context); // is this true by default?

  // hide cursor, report only mouse motion events
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  // enable VSync
  SDL_GL_SetSwapInterval(1);

  if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
    LOG("[ERROR] Couldn't initialize glad");
  } else {
    LOG("[INFO] glad initialized\n");
  }

  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_STENCIL_TEST);
  glViewport(0, 0, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT);
  // debug
  glEnable(GL_DEBUG_OUTPUT);
  glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL,
                        GL_TRUE);

  Gui::InitGuiContext(window, context);
}

void Renderer::ClearContext() {
  Gui::CleanupGui();

  Sausage::WriteShaderMsgsToLogFile();
  SDL_GL_DeleteContext(context);
  SDL_DestroyWindow(window);
  SDL_Quit();
}
void Renderer::_ExecuteCommands() {
  function<void()> f;
  auto commands = gl_commands.PopAll();
  while (!commands.empty()) {
    auto &command = commands.front();
    command.first();
    if (command.second) {
      gl_commands.Push(command);
    }
    commands.pop();
  }
}
