#pragma once

#include "sausage.h"
#include "Settings.h"
#include "Camera.h"
#include "Physics.h"
#include "StateManager.h"

class Controller {
	int screen_x, screen_y;
	bool is_motion_callback_rts = false;
public:
	Camera* camera;
	StateManager* state_manager;
	PhysicsManager* physics_manager;

	Controller(Camera* camera,
		StateManager* state_manager,
		PhysicsManager* physics_manager) : camera{ camera }, state_manager{ state_manager }, physics_manager{ physics_manager } {};

	void ProcessEvent(SDL_Event* e)
	{
		switch (e->type) {
		case SDL_MOUSEMOTION:
		{
			switch (camera->camera_mode)
			{
			case RTS:
				screen_x = e->motion.x;
				screen_y = e->motion.y;
				is_motion_callback_rts = camera->IsCursorOnWindowBorder(screen_x, screen_y) ? true : false;
				break;
			case FREECAM:
				screen_x = e->motion.xrel;
				screen_y = e->motion.yrel;
				is_motion_callback_rts = false;
				break;
			default:
				screen_x = e->motion.x;
				screen_y = e->motion.y;
				is_motion_callback_rts = camera->IsCursorOnWindowBorder(screen_x, screen_y) ? true : false;
				break;
			}
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			physics_manager->ClickRayTest(e->button.x, e->button.y, camera->pos, camera->far_plane - camera->near_plane, camera->projection_view_inverse);
			break;
		}
		case SDL_MOUSEBUTTONUP:
			break;
		case SDL_MOUSEWHEEL:
		{
			camera->MouseWheelCallbackRTS(e->wheel);
			break;
		}
		case SDL_QUIT:
		{
			GameSettings::quit = true;
			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (e->window.event) {
			case SDL_WINDOWEVENT_SIZE_CHANGED:
				GameSettings::SCR_WIDTH = e->window.data1;
				GameSettings::SCR_HEIGHT = e->window.data2;
				glViewport(0, 0, GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT);
				camera->ResizeCallback(GameSettings::SCR_WIDTH, GameSettings::SCR_HEIGHT);
				break;
			}
			break;
		}
		case SDL_KEYDOWN:
		{
			int k = e->key.keysym.sym;
			int s = e->key.keysym.scancode;
			SDL_Keymod mod;
			mod = SDL_GetModState();
			if (k == SDLK_ESCAPE)
				GameSettings::quit = 1;
			camera->KeyCallback(s);
			break;
		}
		case SDL_KEYUP: {
			break;
		}
		}
	}
	void Update()
	{
		if (is_motion_callback_rts) {
			camera->MouseMotionCallback(screen_x, screen_y);
		}
	}
};