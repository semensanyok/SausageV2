#pragma once

#include "sausage.h"
#include <SDL.h>

using namespace glm;

namespace GameSettings {
  inline int SCR_WIDTH = 800;
  inline int SCR_HEIGHT = 800;
  //int SCR_WIDTH = 1920;
  //int SCR_HEIGHT = 1080;

  inline int ray_debug_draw_lifetime_milliseconds = 2000;

  inline float physics_step_multiplier = 0.001f;
  //inline float physics_step_multiplier = 0.01f;

  inline float cam_speed = 1.0f;
  inline bool quit = false;

  inline float MAX_DRAW_DISTANCE = 1000.0;

  inline const vec3 world_right = { 1,0,0 };
  inline const vec3 world_up = { 0,1,0 };
  inline const vec3 world_forward = { 0,0,-1 };
};
namespace CameraSettings {
	inline float sensivity = 0.1f;
	inline float movement_speed = 0.1f;
	inline float scroll_speed = 1.0f;
	inline float mouse_motion_screen_border_velocity = 0.1f;
};
namespace Events {
	inline std::mutex pause_phys_mtx;
	inline std::shared_mutex end_render_frame_mtx;
	inline std::condition_variable_any end_render_frame_event;
};

#ifdef SAUSAGE_PROFILE_ENABLE
namespace ProfTime {
	inline std::chrono::nanoseconds prepare_draws_ns;
	inline std::chrono::nanoseconds render_total_ns;
	inline std::chrono::nanoseconds render_commands_ns;
	inline std::chrono::nanoseconds render_draw_ns;
	inline std::chrono::nanoseconds render_gui_ns;
	inline std::chrono::nanoseconds render_swap_window_ns;
	inline std::chrono::nanoseconds physics_sym_ns;
	inline std::chrono::nanoseconds physics_buf_trans_ns;
	inline std::chrono::nanoseconds total_frame_ns;
	inline std::chrono::nanoseconds physics_debug_draw_world_ns;
	inline std::chrono::nanoseconds anim_update_ns;
}
#endif

inline std::thread::id main_thread_id;

namespace KeyboardLayout {
	inline SDL_Scancode Up = SDL_SCANCODE_W;
	inline SDL_Scancode Left = SDL_SCANCODE_A;
	inline SDL_Scancode Right = SDL_SCANCODE_D;
	inline SDL_Scancode Down = SDL_SCANCODE_S;
	inline SDL_Scancode LShift = SDL_SCANCODE_LSHIFT;
	inline SDL_Scancode ChangeCamera = SDL_SCANCODE_Z;
  inline SDL_Scancode PauseMenu = SDL_SCANCODE_ESCAPE;
};

namespace KeyboardEvents {
	inline std::set<int> keys_pressed;
};

#define SAUSAGE_DEBUG_BUILD

#ifdef SAUSAGE_DEBUG_BUILD
#include <assert.h>
#define SAUSAGE_DEBUG_ASSERT(x) assert(x);
#else
#define SAUSAGE_DEBUG_ASSERT(x) //
#endif  // 1
