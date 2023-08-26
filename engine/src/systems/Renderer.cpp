#include "Renderer.h"
#include "SceneManager.h"
#include "DrawCallManager.h"

using namespace std;

void Renderer::Render(Camera* camera) {
  IF_PROFILE_ENABLED(auto proft1 = chrono::steady_clock::now(););
  _ExecuteCommands();
  IF_PROFILE_ENABLED(auto proft2 = chrono::steady_clock::now(););
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  GPUSynchronizer::GetInstance()->SyncGPU();
  {
    SceneManager::GetInstance()->PreDraw();
    buffer_manager->PreDraw();
    //command_buffers_manager->PreDraw();
    for (auto order_shader : draw_calls) {
      //if (order_shader.first != DrawOrder::MESH_STATIC) {
      //  continue;
      //}
      if (order_shader.second.empty()) {
        continue;
      }
      {
        DEBUG_EXPR(CheckGLError());
        vertex_attributes->BindVAO(GetVertexTypeByDrawOrder(order_shader.first));
        //DEBUG_EXPR(CheckGLError());
        for (auto draw : order_shader.second) {
          lock_guard l(draw->mtx);
          // NOTE: currently draw->PreDraw() cleans command array
          unsigned int command_count = draw->GetCommandCount();
          if (draw->is_enabled && command_count > 0) {
            draw->PreDraw();
            command_buffers_manager->UnmapBuffer(draw->command_buffer);
            glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw->command_buffer->ptr->buffer_id);
            glUseProgram(draw->shader->id);
            //DEBUG_EXPR(CheckGLError());
            draw->shader->SetUniforms();
            //DEBUG_EXPR(CheckGLError());
            // with core profile cannot issue call without DRAW_INDIRECT_BUFFER
            // must enable compatibility profile
            // otherwise RAM command array doesnt work
            // but what if dummy buffer is bound ???
            glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT,
                                        //draw->commands.data(),
                                        nullptr,
                                        command_count, 0);
            DEBUG_EXPR(CheckGLError());
            draw->PostDraw();
            command_buffers_manager->MapBuffer(draw->command_buffer);
            //DEBUG_EXPR(CheckGLError());
          }
        }
      }
      DEBUG_EXPR(CheckGLError());
    }
    buffer_manager->PostDraw();
    //command_buffers_manager->PostDraw();
    spatial_manager->PostDraw();
    dc_manager->ResetFrameCommands();
    GPUSynchronizer::GetInstance()->PostDraw();
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
