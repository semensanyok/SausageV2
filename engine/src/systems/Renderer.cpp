#include "Renderer.h"

using namespace std;

void Renderer::Render(Camera* camera) {
  IF_PROFILE_ENABLED(auto proft1 = chrono::steady_clock::now(););
  _ExecuteCommands();
  IF_PROFILE_ENABLED(auto proft2 = chrono::steady_clock::now(););
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  {
    for (auto order_shader : draw_calls) {
      if (order_shader.second.empty()) {
        continue;
      }
      {
        buffer->PreDraw();
        for (auto draw : order_shader.second) {
          if (draw->is_enabled && draw->GetCommandCount() > 0) {
            glUseProgram(draw->shader->id);
            draw->shader->SetUniforms();
            glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT, nullptr,
                                        draw->GetCommandCount(), draw->GetBaseOffset());
          }
        }
        buffer->PostDraw();
      }
      CheckGLError();
    }
  }
  IF_PROFILE_ENABLED(auto proft3 = chrono::steady_clock::now(););
  Gui::RenderGui(context_manager->window, camera);
  IF_PROFILE_ENABLED(auto proft4 = chrono::steady_clock::now(););
  SDL_GL_SwapWindow(context_manager->window);
  IF_PROFILE_ENABLED(
    auto proft5 = chrono::steady_clock::now();
  ProfTime::render_total_ns = proft5 - proft1;
  ProfTime::render_commands_ns = proft2 - proft1;
  ProfTime::render_draw_ns = proft3 - proft2;
  ProfTime::render_gui_ns = proft4 - proft3;
  ProfTime::render_swap_window_ns = proft5 - proft4;
  );
  Events::end_render_frame_event.notify_all();
}

void Renderer::AddGlCommand(function<void()> &f, bool is_persistent) {
  gl_commands.Push({ f, is_persistent });
}

bool Renderer::AddDraw(DrawCall *draw, DrawOrder draw_order) {
  draw_calls[draw_order].insert(draw);
  return true;
}

bool Renderer::RemoveDraw(DrawCall *draw, DrawOrder draw_order) {
  auto calls = draw_calls.find(draw_order);
  if (calls == draw_calls.end()) {
    LOG("Unable to remove draw, DrawOrder not found");
    return false;
  }
  auto erased = calls->second.erase(draw);
  if (erased < 1) {
    LOG("Unable to remove draw. Active DrawCall not found, possibly inactive");
    return false;
  }
  return true;
}

void Renderer::_ExecuteCommands() {
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
