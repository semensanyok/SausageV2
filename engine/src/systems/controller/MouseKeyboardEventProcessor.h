#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "ScreenOverlayManager.h"
#include "BufferManager.h"

class ControllerEventProcessor : public SausageSystem {
protected:
  Camera* camera;
  ScreenOverlayManager* screen_overlay_manager;
public:
  inline ControllerEventProcessor(
    Camera* camera,
    ScreenOverlayManager* screen_overlay_manager) :
    camera{ camera },
    screen_overlay_manager{ screen_overlay_manager } {};
  inline ~ControllerEventProcessor() {};
  virtual void KeyDown(int key) {};
  virtual void KeyUp(int key) {};
  virtual void MouseButtonUp(SDL_MouseButtonEvent& e) {};
  virtual void MouseButtonDown(SDL_MouseButtonEvent& e) {};
  virtual void MouseMotion(SDL_MouseMotionEvent& e) {};
  virtual void MouseWheelCallback(SDL_MouseWheelEvent& mw_event) {};
  virtual void Update() {};
};

class ControllerEventProcessorEditor : public ControllerEventProcessor, public SausageSystem {
	int screen_x, screen_y;
	bool is_motion_callback_rts_continious_scroll = false;
  BufferStorage* buffer_storage;
  bool is_mouse_left_click = false;
  bool is_mouse_left_pressed = false;
  
public:
	ControllerEventProcessorEditor(Camera* camera,
    ScreenOverlayManager* screen_overlay_manager,
    BufferManager* buffer_manager);
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
