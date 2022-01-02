#include "MouseKeyboardEventProcessor.h"

ControllerEventProcessorEditor::ControllerEventProcessorEditor(
  Camera* camera,
  ScreenOverlayManager* screen_overlay_manager,
  BufferManager* buffer_manager) :
  ControllerEventProcessor(camera, screen_overlay_manager), buffer_storage{ buffer_manager->storage } {
}

ControllerEventProcessorEditor::~ControllerEventProcessorEditor() {
}

void ControllerEventProcessorEditor::KeyDown(int key)
{
	camera->KeyCallback(key);
  screen_overlay_manager->KeyCallback(key);
}

void ControllerEventProcessorEditor::KeyUp(int key)
{

}

void ControllerEventProcessorEditor::MouseButtonUp(SDL_MouseButtonEvent& e)
{
  if (e.which == SDL_BUTTON_LEFT) {
    is_mouse_left_pressed = false;
  }
}

void ControllerEventProcessorEditor::MouseButtonDown(SDL_MouseButtonEvent& e)
{
  if (e.which == SDL_BUTTON_LEFT) {
    is_mouse_left_pressed = true;
    is_mouse_left_click = true;
  }
  screen_overlay_manager->OnClick(e.x, e.y);
}

void ControllerEventProcessorEditor::MouseMotion(SDL_MouseMotionEvent& e)
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
	}
}

void ControllerEventProcessorEditor::Update() {
	if (is_motion_callback_rts_continious_scroll) {
		camera->MouseMotionCallback(screen_x, screen_y);
	}
  screen_overlay_manager->OnHover(screen_x, screen_y);
  buffer_storage->BufferUniformDataController(screen_x, screen_y, (int)is_mouse_left_pressed, (int)is_mouse_left_click);
  is_mouse_left_click = false;
}
