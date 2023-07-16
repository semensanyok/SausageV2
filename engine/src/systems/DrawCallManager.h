#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Logging.h"
#include "OverlayStruct.h"

using namespace std;

class StateManager;
class ShaderManager;
class Shader;
class DrawCall;

class DrawCallManager {
public:

  DrawCall* mesh_dc;

  DrawCall* back_ui_dc;
  DrawCall* font_ui_dc;
  DrawCall* mesh_static_dc;
  DrawCall* terrain_dc;

  //DrawCall* overlay_3d_dc;

  DrawCall* physics_debug_dc;

  int total_draw_calls = 0;

  DrawCallManager(
    ShaderManager* shader_manager,
    StateManager* state_manager
  );

  void ResetFrameCommands();
private:
  DrawCall* CreateDrawCall(Shader* shader, GLenum mode, bool is_enabled);
};
