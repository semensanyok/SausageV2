#include "DrawCallManager.h"
#include "DrawCall.h"
#include "ShaderManager.h"
#include "StateManager.h"

DrawCallManager::DrawCallManager(
  ShaderManager* shader_manager,
  StateManager* state_manager,
  CommandBuffersManager* cbm
) {
  font_ui_dc = CreateDrawCall(
        shader_manager->all_shaders->font_ui,
        GL_TRIANGLES,
        false,
        cbm->command_buffers.font_ui
  );

  back_ui_dc = CreateDrawCall(
      shader_manager->all_shaders->back_ui,
      GL_TRIANGLES,
      false,
        cbm->command_buffers.back_ui
  );

  //overlay_3d_dc = CreateDrawCall(
  //shader_manager->all_shaders->overlay_3d,
  //GL_TRIANGLES,
  //true
  //);

  outline_dc = CreateDrawCall(
      shader_manager->all_shaders->outline,
      GL_LINES,
      state_manager->phys_debug_draw,
        cbm->command_buffers.outline
  );

  mesh_dc = CreateDrawCall(
      shader_manager->all_shaders->blinn_phong,
      GL_TRIANGLES,
      true,
        cbm->command_buffers.blinn_phong
  );

  mesh_static_dc = CreateDrawCall(
      shader_manager->all_shaders->mesh_static,
      GL_TRIANGLES,
      true,
        cbm->command_buffers.mesh_static
  );

  terrain_dc = CreateDrawCall(
      shader_manager->all_shaders->terrain,
      GL_TRIANGLES,
      true,
        cbm->command_buffers.terrain
  );
}

void DrawCallManager::ResetFrameCommands() {
  mesh_dc->Reset();
  mesh_static_dc->Reset();
  terrain_dc->Reset();
  outline_dc->Reset();
}

DrawCall* DrawCallManager::CreateDrawCall(Shader* shader, GLenum mode, bool is_enabled, CommandBuffer* command_buffer)
{
  return new DrawCall(total_draw_calls++, shader, mode, command_buffer, is_enabled);
}
