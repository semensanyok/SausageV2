#include "DrawCallManager.h"
#include "DrawCall.h"
#include "ShaderManager.h"
#include "StateManager.h"

DrawCallManager::DrawCallManager(
  ShaderManager* shader_manager,
  StateManager* state_manager
) {
    font_ui_dc = CreateDrawCall(
        shader_manager->all_shaders->font_ui,
        GL_TRIANGLES,
        false
    );

    back_ui_dc = CreateDrawCall(
        shader_manager->all_shaders->back_ui,
        GL_TRIANGLES,
        false
    );

    //overlay_3d_dc = CreateDrawCall(
    //shader_manager->all_shaders->overlay_3d,
    //GL_TRIANGLES,
    //true
    //);

    outline_dc = CreateDrawCall(
        shader_manager->all_shaders->outline,
        GL_LINES,
        state_manager->phys_debug_draw
    );

    mesh_dc = CreateDrawCall(
        shader_manager->all_shaders->blinn_phong,
        GL_TRIANGLES,
        true
    );

    mesh_static_dc = CreateDrawCall(
        shader_manager->all_shaders->mesh_static,
        GL_TRIANGLES,
        true
    );

    terrain_dc = CreateDrawCall(
        shader_manager->all_shaders->terrain,
        GL_TRIANGLES,
        true
    );
}

void DrawCallManager::ResetFrameCommands() {
  mesh_dc->Reset();
  mesh_static_dc->Reset();
  terrain_dc->Reset();
  outline_dc->Reset();
}

DrawCall* DrawCallManager::CreateDrawCall(Shader* shader, GLenum mode, bool is_enabled)
{
  return new DrawCall(total_draw_calls++, shader, mode, is_enabled);
}
