#pragma once

#include "../sausage.h"
#include "Camera.h"
#include "../Settings.h"

class ControllerEventProcessor {
protected:
	Camera* camera;
public:
	ControllerEventProcessor(Camera* camera) : camera { camera } {};
	~ControllerEventProcessor() {};
	virtual void KeyDown(int key) {};
	virtual void KeyUp(int key) {};
	virtual void MouseButtonUp(SDL_MouseButtonEvent& e) {};
	virtual void MouseButtonDown(SDL_MouseButtonEvent& e) {};
	virtual void MouseMotion(SDL_MouseMotionEvent& e) {};
	virtual void Update() {};
};

class ControllerEventProcessorEditor : public ControllerEventProcessor {
	int screen_x, screen_y;
	bool is_motion_callback_rts_continious_scroll = false;
public:
	ControllerEventProcessorEditor(Camera* camera) : ControllerEventProcessor(camera) {
	}
	~ControllerEventProcessorEditor() {
	}
	void KeyDown(int key) 
	{
		camera->KeyCallback(key);
	};
	void KeyUp(int key) 
	{

	};
	void MouseButtonUp(SDL_MouseButtonEvent& e) 
	{
	};
	void MouseButtonDown(SDL_MouseButtonEvent& e) 
	{
	};
	void MouseMotion(SDL_MouseMotionEvent& e)
	{
		switch (camera->camera_mode)
		{
		case RTS:
			screen_x = e.x;
			screen_y = e.y;
			is_motion_callback_rts_continious_scroll = camera->IsCursorOnWindowBorder(screen_x, screen_y) ? true : false;
			break;
		case FREE:
			screen_x = e.xrel;
			screen_y = e.yrel;
			is_motion_callback_rts_continious_scroll = false;
			if (KeyboardEvents::keys_pressed.find(KeyboardLayout::LShift) != KeyboardEvents::keys_pressed.end()) {
				SDL_ShowCursor(SDL_DISABLE);
				SDL_SetRelativeMouseMode(SDL_TRUE);
				camera->MouseMotionCallback(screen_x, screen_y);
			}
			else {
				SDL_ShowCursor(SDL_ENABLE);
				SDL_SetRelativeMouseMode(SDL_FALSE);
			}
			break;
		default:
			screen_x = e.x;
			screen_y = e.y;
			is_motion_callback_rts_continious_scroll = camera->IsCursorOnWindowBorder(screen_x, screen_y) ? true : false;
			break;
		}
	};
	void Update() {
		if (is_motion_callback_rts_continious_scroll) {
			camera->MouseMotionCallback(screen_x, screen_y);
		}
	};
};

class ControllerEventProcessorPlayer : ControllerEventProcessor {
	ControllerEventProcessorPlayer(Camera* camera) : ControllerEventProcessor(camera) {
	}
	~ControllerEventProcessorPlayer() {};
};