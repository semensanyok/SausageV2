#pragma once

#include "sausage.h"
#include "Structures.h"
#include "Macros.h"

using namespace glm;

namespace AnchorRelativeToNodePosition {
  enum AnchorRelativeToNodePosition {
    LeftBottom,
    LeftTop,
    RightBottom,
    RightTop,
  };
}

struct UINodePosition {
  Point anchor_position;
  AnchorRelativeToNodePosition::AnchorRelativeToNodePosition anchor_relative_to_node_position;
  int width;
  int height;
};

class PauseMenuSettings {
public:
  int button_font_size;
  int back_indent;
  int button_width;
  int button_height;
  vec3 text_color;
  vec3 back_color;
  PauseMenuSettings(
    int button_font_size,
    int back_indent,
    vec3 text_color,
    vec3 back_color) :
    button_font_size{ button_font_size },
    back_indent{ back_indent },
    text_color{ text_color },
    back_color{ back_color } {
    button_width = button_font_size * 4 + 2 * back_indent;
    button_height = button_font_size + 2 * back_indent;
  }
};


namespace ScreenOverlayManagerInternals {
  const vector<string> buttons = {
    "quit",
    "settings",
    "load",
    "save",
    "resume",
  };
}

using namespace ScreenOverlayManagerInternals;

// to allocate constant instance slot.
// BufferStorage#AllocateMeshUniformSlot
//
// add all ui elements in this header file,
// add their count to this function
constexpr unsigned int GetNumDrawCommandsForFontDrawCall() {
  //return buttons.size();
  //unsigned int res = 5;
  //DEBUG_ASSERT(res == buttons.size());
  return 5;
}
// if anywhere in future start using instanced draws for ui elements
// - will be larger then GetNumDrawCommandsForFontDrawCall()
constexpr unsigned int GetNumDrawCommandsForFontDrawCall_InstancedMeshes() {
  return GetNumDrawCommandsForFontDrawCall();
}

// for background canvases of buttons text 
constexpr unsigned int GetNumDrawCommandsForBackDrawCall() {
  return buttons.size();
}

/*
* main part defined in BufferSettings.h
*/
namespace BufferSettings {
  const unsigned long TEXTURES_SINGLE_FONT = 128;
  const unsigned long MAX_3D_OVERLAY_COMMANDS = 1000; // TODO: figure out correct size
  const unsigned long MAX_3D_OVERLAY_INSTANCES = MAX_3D_OVERLAY_COMMANDS
    //* 10
    ; // TODO: figure out correct size
  const unsigned long MAX_FONT_TEXTURES = 1 * TEXTURES_SINGLE_FONT;

  constexpr unsigned long MAX_UI_COMMANDS = GetNumDrawCommandsForFontDrawCall();
  constexpr unsigned long MAX_UI_INSTANCES = GetNumDrawCommandsForFontDrawCall_InstancedMeshes();
};
