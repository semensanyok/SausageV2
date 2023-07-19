#pragma once

#include "sausage.h"
#include "Macros.h"
#include "Logging.h"
#include "OverlayStruct.h"
#include "MeshDataStruct.h"

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

  DrawCall* outline_dc;

  int total_draw_calls = 0;

  DrawCallManager(
    ShaderManager* shader_manager,
    StateManager* state_manager
  );

  void ResetFrameCommands();

  template <typename MESH_TYPE>
  DrawCall* GetDrawCall() {
    //return nullptr;
    throw runtime_error("Not implemented");
  };
  template <>
  DrawCall* GetDrawCall<MeshData>() {
    return mesh_dc;
  };
  template <>
  DrawCall* GetDrawCall<MeshDataStatic>() {
    return mesh_static_dc;
  };
  template <>
  DrawCall* GetDrawCall<MeshDataTerrain>() {
    return terrain_dc;
  };
  template <>
  DrawCall* GetDrawCall<MeshDataOutline>() {
    return outline_dc;
  };
private:
  DrawCall* CreateDrawCall(Shader* shader, GLenum mode, bool is_enabled);
};
