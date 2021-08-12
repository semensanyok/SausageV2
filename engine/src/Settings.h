#pragma once

#include "sausage.h"
#include "OpenGLHelpers.h"

namespace GameSettings {
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 800;
	//int SCR_WIDTH = 1920;
	//int SCR_HEIGHT = 1080;

	int ray_debug_draw_lifetime_milliseconds = 2000;
	float physics_step_multiplier = 0.00001f;
	bool quit = false;
	bool phys_debug_draw = true;
}
namespace CameraSettings {
	float sensivity = 0.1f;
	float movement_speed = 0.1f;
	float scroll_speed = 1.0f;
	float mouse_motion_screen_border_velocity = 0.1f;
}
namespace Events {
	std::mutex pause_phys_mtx;

	std::shared_mutex end_render_frame_mtx;
	std::condition_variable_any end_render_frame_event;
}

namespace BufferSettings {
	const unsigned long MAX_VERTEX = 1000000;
	const unsigned long MAX_INDEX = 10000000;
	const unsigned long MAX_COMMAND = 1000;
	const unsigned long MAX_TRANSFORM = 4000;
	const unsigned long MAX_BONES = 100000;
	const unsigned long MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;
	const unsigned long MAX_TEXTURES = MAX_COMMAND;
	const unsigned long MAX_LIGHTS = 1000;
	// reserve last 1/4 of buffer for debug vertices;
	namespace Margins {
		const float MESH_DATA_VERTEX_PART = 0.65;
		const float MESH_DATA_INDEX_PART = 0.65;
#ifdef SAUSAGE_DEBUG_DRAW_PHYSICS
		const float DEBUG_PHYS_VERTEX_PART = 0.2;
		const float DEBUG_PHYS_INDEX_PART = 0.2;
#else
		const float DEBUG_PHYS_VERTEX_PART = 0;
		const float DEBUG_PHYS_INDEX_PART = 0;
#endif
		const float FONT_VERTEX_PART = 0.15;
		const float FONT_INDEX_PART = 0.15;
	}
}

#ifdef SAUSAGE_PROFILE_ENABLE
namespace ProfTime {
	std::chrono::nanoseconds prepare_draws_ns;
	
	std::chrono::nanoseconds render_total_ns;
	std::chrono::nanoseconds render_commands_ns;
	std::chrono::nanoseconds render_draw_ns;
	std::chrono::nanoseconds render_gui_ns;
	std::chrono::nanoseconds render_swap_window_ns;

	std::chrono::nanoseconds physics_sym_ns;
	std::chrono::nanoseconds physics_buf_trans_ns;
	std::chrono::nanoseconds total_frame_ns;
	std::chrono::nanoseconds physics_debug_draw_world_ns;

	std::chrono::nanoseconds anim_update_ns;
}
#endif

std::thread::id main_thread_id;

// note: gl.... commands are valid only in render thread.
// any calls from other threads results in error.
bool CheckGLError(const std::source_location& location) {
	if (std::this_thread::get_id() != main_thread_id) {
		return false;
	}
	int err = glGetError();
	if (err != GL_NO_ERROR) {
		//LOG((ostringstream() << "GL error: '" << glGetErrorString(err)
		//		<< "' at: "
		//		<< location.file_name() << "("
		//		<< location.line() << ":"
		//		<< location.column() << ")#"
		//		<< location.function_name()).str());
		std::cout << "GL error: '" << glGetErrorString(err)
			<< "' at: "
			<< location.file_name() << "("
			<< location.line() << ":"
			<< location.column() << ")#"
			<< location.function_name() << std::endl;
		return true;
	}
	return false;
}

namespace KeyboardLayout {
	SDL_Scancode Up = SDL_SCANCODE_W;
	SDL_Scancode Left = SDL_SCANCODE_A;
	SDL_Scancode Right = SDL_SCANCODE_D;
	SDL_Scancode Down = SDL_SCANCODE_S;
	SDL_Scancode LShift = SDL_SCANCODE_LSHIFT;
	SDL_Scancode ChangeCamera = SDL_SCANCODE_Z;

}

namespace KeyboardEvents {
	std::set<int> keys_pressed;
}