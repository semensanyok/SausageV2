#pragma once

#include "sausage.h"

namespace GameSettings {
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 800;
	//int SCR_WIDTH = 1920;
	//int SCR_HEIGHT = 1080;

	float delta_time = 0;
	float last_ticks = 0;
	uint32_t milliseconds_since_start = 0;
	int ray_debug_draw_lifetime_milliseconds = 2000;
	float physics_step_multiplier = 0.0001f;
	bool quit = false;

	std::mutex end_render_frame_mtx;
	std::condition_variable end_render_frame_event;
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
	const unsigned long DEBUG_VERTEX_OFFSET = MAX_VERTEX - MAX_VERTEX * 0.25;
	const unsigned long DEBUG_INDEX_OFFSET = MAX_INDEX - MAX_INDEX * 0.25;
}

#ifdef SAUSAGE_PROFILE_ENABLE
namespace ProfTime {
	std::chrono::nanoseconds prepare_draws_ns;
	std::chrono::nanoseconds render_ns;
	std::chrono::nanoseconds physics_sym_ns;
	std::chrono::nanoseconds physics_buf_trans_ns;
	std::chrono::nanoseconds total_frame_ns;
}
#endif
