#pragma once

#include "sausage.h"
#include "OpenGLHelpers.h"

namespace GameSettings {
	inline int SCR_WIDTH = 800;
	inline int SCR_HEIGHT = 800;
	//int SCR_WIDTH = 1920;
	//int SCR_HEIGHT = 1080;

	inline int ray_debug_draw_lifetime_milliseconds = 2000;
	inline float physics_step_multiplier = 0.00001f;
	inline bool quit = false;
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
	inline bool phys_debug_draw = true;
#else
	inline bool phys_debug_draw = false;
#endif
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

namespace BufferSettings {
	const unsigned long MAX_VERTEX = 1000000;
	const unsigned long MAX_INDEX = 10000000;
	const unsigned long MAX_COMMAND = 1000;
	const unsigned long MAX_TRANSFORM = 4000;
	const unsigned long MAX_BONES = 100000;
	const unsigned long MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;
	const unsigned long MAX_TEXTURES = MAX_COMMAND;
	const unsigned long TEXTURES_SINGLE_FONT = 128;
	const unsigned long MAX_LIGHTS = 1000;
	//FONT
	const unsigned long MAX_FONT_TRANSFORM = 4000;
	const unsigned long MAX_FONT_TRANSFORM_OFFSET = MAX_FONT_TRANSFORM * 10;
	const unsigned long MAX_FONT_TEXTURES = 1 * TEXTURES_SINGLE_FONT;

	const unsigned long MAX_FONT_UI_TRANSFORM = 400;
    const unsigned long MAX_FONT_UI_TRANSFORM_OFFSET = MAX_FONT_UI_TRANSFORM * 10;
	// parts of buffer;
	namespace Margins {
		const float MESH_DATA_VERTEX_PART = 0.65;
		const float MESH_DATA_INDEX_PART = 0.65;
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		const float DEBUG_PHYS_VERTEX_PART = 0.20;
		const float DEBUG_PHYS_INDEX_PART = 0.20;
#else
		const float DEBUG_PHYS_VERTEX_PART = 0;
		const float DEBUG_PHYS_INDEX_PART = 0;
#endif
		const float FONT_VERTEX_PART = 0.05;
		const float FONT_INDEX_PART = 0.05;
	}
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
};

namespace KeyboardEvents {
	inline std::set<int> keys_pressed;
};
