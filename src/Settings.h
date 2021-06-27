#pragma once

#include "sausage.h"

namespace GameSettings {
	int SCR_WIDTH = 800;
	int SCR_HEIGHT = 800;
	//int SCR_WIDTH = 1920;
	//int SCR_HEIGHT = 1080;

	uint32_t milliseconds_since_start = 0;
	int ray_debug_draw_lifetime_milliseconds = 2000;
	float physics_step_multiplier = 0.0001f;
}

namespace BufferSettings {
	const unsigned long MAX_VERTEX = 1000000;
	const unsigned long MAX_INDEX = 100000;
	const unsigned long MAX_COMMAND = 1000;
	const unsigned long MAX_TRANSFORM = 4000;
	const unsigned long MAX_BONES = 100000;
	const unsigned long MAX_TRANSFORM_OFFSET = MAX_TRANSFORM * 10;
	const unsigned long MAX_TEXTURES = MAX_COMMAND;
	const unsigned long MAX_LIGHTS = 1000;
}