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