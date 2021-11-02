#pragma once

#include "sausage.h"
#include "Settings.h"
#include "MouseKeyboardEventProcessor.h"
#include "Camera.h"
#include "Physics.h"
#include "StateManager.h"

class Controller {
public:
	Camera* camera;
	ControllerEventProcessor* processor;
	StateManager* state_manager;
	PhysicsManager* physics_manager;

	Controller(Camera* camera,
		StateManager* state_manager,
		PhysicsManager* physics_manager,
		ControllerEventProcessor* processor) : camera{ camera }, state_manager{ state_manager }, physics_manager{ physics_manager }, processor{ processor }{};

	void ProcessEvent(SDL_Event* e)
	{
		switch (e->type) {
		case SDL_MOUSEMOTION:
		{
			processor->MouseMotion(e->motion);
			break;
		}
		case SDL_MOUSEBUTTONDOWN:
		{
			physics_manager->ClickRayTestClosest(e->button.x, e->button.y, camera->pos, camera->far_plane - camera->near_plane, camera->projection_view_inverse);
			processor->MouseButtonDown(e->button);
			break;
		}
		case SDL_MOUSEBUTTONUP:
		{
			processor->MouseButtonUp(e->button);
			break;
		}
		case SDL_MOUSEWHEEL:
		{
			camera->MouseWheelCallback(e->wheel);
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
			int s = e->key.keysym.scancode;
			KeyboardEvents::keys_pressed.insert(s);
			if (s == SDL_SCANCODE_ESCAPE)
				GameSettings::quit = 1;
			processor->KeyDown(s);
			break;
		}
		case SDL_KEYUP: {
			int s = e->key.keysym.scancode;
			KeyboardEvents::keys_pressed.erase(s);
			processor->KeyUp(s);
			break;
		}
		}
	}
	void Update()
	{
	}
};
