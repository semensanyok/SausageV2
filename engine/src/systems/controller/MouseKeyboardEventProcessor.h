#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"

class ControllerEventProcessor {
protected:
	Camera* camera;
public:
	inline ControllerEventProcessor(Camera* camera) : camera { camera } {};
	inline ~ControllerEventProcessor() {};
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
	ControllerEventProcessorEditor(Camera* camera);
	~ControllerEventProcessorEditor();
	void KeyDown(int key);
	void KeyUp(int key);
	void MouseButtonUp(SDL_MouseButtonEvent& e);
	void MouseButtonDown(SDL_MouseButtonEvent& e);
	void MouseMotion(SDL_MouseMotionEvent& e);
	void Update();
};

//class ControllerEventProcessorPlayer : ControllerEventProcessor {
//	ControllerEventProcessorPlayer(Camera* camera) : ControllerEventProcessor(camera) {
//	}
//	~ControllerEventProcessorPlayer() {};
//};