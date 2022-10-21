#pragma once

#include "sausage.h"
#include "Structures.h"

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

// to allocate constant instance slot.
// BufferStorage#AllocateInstancesSlot
//
// add all ui elements in this header file,
// add their count to this function
constexpr unsigned int GetNumDrawCommandsForFontDrawCall() {
  return buttons.size();
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
  const unsigned long MAX_3D_OVERLAY_TRANSFORM = 4000;
  const unsigned long MAX_3D_OVERLAY_TRANSFORM_OFFSET = MAX_3D_OVERLAY_TRANSFORM * 10;
  const unsigned long MAX_FONT_TEXTURES = 1 * TEXTURES_SINGLE_FONT;

  const unsigned long MAX_UI_UNIFORM_TRANSFORM = GetNumDrawCommandsForFontDrawCall();
  const unsigned long MAX_UI_UNIFORM_OFFSET = GetNumDrawCommandsForFontDrawCall_InstancedMeshes();
};
