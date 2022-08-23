#include "Renderer.h"
#include "Macros.h"

using namespace std;

void Renderer::Render(Camera *camera) {
  IF_PROFILE_ENABLED(auto proft1 = chrono::steady_clock::now(););
  _ExecuteCommands();
  IF_PROFILE_ENABLED(auto proft2 = chrono::steady_clock::now(););
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  {
    for (auto buffer_shader_order : buffer_to_draw_call) {
      for (auto buffer_shader : buffer_shader_order.second) {
        if (buffer_shader.second.empty()) {
          continue;
        }
        auto draw_call_iter = buffer_shader.second.begin();
        auto buffer_consumer = (*draw_call_iter)->buffer;
        auto used_buffers = buffer_consumer->GetUsedBuffers();
        draw_call_iter++;
        while (draw_call_iter != buffer_shader.second.end()) {
          used_buffers |= (*draw_call_iter)->buffer->GetUsedBuffers();
          draw_call_iter++;
        }
        {
          buffer_consumer->PreDraw(used_buffers);
          for (auto draw : buffer_shader.second) {
            if (draw->command_count > 0) {
              glUseProgram(draw->shader->id);
              glBindBuffer(GL_DRAW_INDIRECT_BUFFER, draw->command_buffer->id);
              draw->shader->SetUniforms();
              glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT, nullptr,
                                          draw->command_count, 0);
            }
          }
          buffer_consumer->PostDraw();
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
  }
  Events::end_render_frame_event.notify_all();
}

void Renderer::RemoveBuffer(BufferStorage *buffer) {
  auto buf = buffer_to_draw_call.find(buffer->id);
  if (buf != buffer_to_draw_call.end()) {
    buffer_to_draw_call.erase(buffer->id);
  }
}

void Renderer::AddGlCommand(function<void()> &f, bool is_persistent) {
  gl_commands.Push(pair(f, is_persistent));
}

bool Renderer::AddDraw(DrawCall *draw, DrawOrder::DrawOrder draw_order) {
  // MAYBE NEEDED SAFE CHECK  
  //if (!shaders.contains(draw->shader->id)) {
  //  LOG((ostringstream() << "Unable to add draw for unregistered shader: "
  //                       << draw->shader->id
  //                       << " vs:" << draw->shader->vertex_path
  //                       << " fs:" << draw->shader->fragment_path)
  //          .str());
  //  return false;
  //}

  // if (buf_shad_ids.contains(buf_shad_id)) {
  //	LOG((ostringstream() << "Draw for shader: " << draw->shader->id << "
  //buffer:" << draw->buffer->id << "already exists").str()); 	return false;
  //}
  buffer_to_draw_call[draw->buffer->GetBufferId()][draw_order].push_back(draw);
  return true;
}

bool Renderer::RemoveDraw(DrawCall *draw, DrawOrder::DrawOrder draw_order) {
  auto btd_ptr = buffer_to_draw_call.find(draw->buffer->GetBufferId());
  if (btd_ptr == buffer_to_draw_call.end()) {
    LOG("Unable to remove draw, buffer not found");
    return false;
  }
  if (btd_ptr->second.find(draw_order) == btd_ptr->second.end()) {
    LOG((ostringstream()
      << "Unable to remove draw, draw order '"
      << draw_order << "' for buffer not found").str());
    return false;
  }
  auto &draws = btd_ptr->second[draw_order];
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
