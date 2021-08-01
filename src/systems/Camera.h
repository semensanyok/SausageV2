#pragma once

#include "sausage.h"
#include "Logging.h"

using namespace std;
using namespace glm;

enum CameraMode {
	RTS,
	FREECAM
};

class Camera
{
private:
	friend class Gui;
	friend class Renderer;
	friend class Controller;
	friend class Scene;

	void _UpdateMatrices() {
		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
		this->projection_view = projection_matrix * view_matrix;
		this->projection_view_inverse = inverse(view_matrix) * inverse(projection_matrix);
	}
	mat4 view_matrix;
	mat4 projection_matrix;
	mat4 projection_view;
	mat4 projection_view_inverse;
	vec3 pos;
	vec3 up;
	vec3 world_up;
	vec3 right;
	vec3 direction;
	float yaw_angle;
	float pitch_angle;

	float FOV;
	float width;
	float height;
	float near_plane;
	float far_plane;

	int screen_border = 20;

	// per frame variables
	float velocity;
	float delta_time;
public:
	bool is_update_need = false;
	CameraMode camera_mode = CameraMode::RTS;

	Camera(float FOV, 
		float width,
		float height,
		float near_plane,
		float far_plane,
		vec3 pos,
		float yaw_angle = 0.0f,
		float pitch_angle = 0.0f,
		vec3 up = vec3(0.0f, 1.0f, 0.0f),
		vec3 world_up = vec3(0.0f, 1.0f, 0.0f)
	) {
		this->pos = pos;
		this->FOV = FOV;
		this->width = width;
		this->height = height;
		this->near_plane = near_plane;
		this->far_plane = far_plane;
		this->yaw_angle = yaw_angle;
		this->pitch_angle = pitch_angle;
		this->world_up = world_up;
		this->up = up;
		this->projection_matrix = perspective(radians(this->FOV), (float)this->width / (float)this->height, this->near_plane, this->far_plane);
		UpdateCameraFrontUpRight();
		_UpdateMatrices();
	};
	~Camera() {};
	void UpdateCameraFrontUpRight() {
		vec3 target = vec3(sin(radians(yaw_angle)) * cos(radians(pitch_angle)), sin(radians(pitch_angle)), -cos(radians(yaw_angle)) * cos(radians(pitch_angle)));
		direction = normalize(target);
		right = normalize(cross(target, world_up));
		up = normalize(cross(right, target));
	}
	void MouseWheelCallback(SDL_MouseWheelEvent& mw_event) {
		switch (camera_mode)
		{
		case RTS:
			MouseWheelCallbackRTS(mw_event);
			break;
		case FREECAM:
			MouseWheelCallbackFreeCam(mw_event);
			break;
		default:
			break;
		}
		this->pos.y -= (mw_event.y * CameraSettings::scroll_speed);

		is_update_need = true;
	};
	void MouseWheelCallbackRTS(SDL_MouseWheelEvent& mw_event) {
		this->pos.y -= (mw_event.y * CameraSettings::scroll_speed);
		
		is_update_need = true;
	};
	void MouseWheelCallbackFreeCam(SDL_MouseWheelEvent& mw_event) {
		this->pos += direction;

		is_update_need = true;
	};
	void PreUpdate(float delta_time) {
		this->velocity = delta_time * CameraSettings::movement_speed;
		this->delta_time = delta_time;
	}
	void Update() {
		if (is_update_need) {
			is_update_need = false;
			UpdateCameraFrontUpRight();
			_UpdateMatrices();
		}
	}
	void KeyCallback(int scan_code) {
		switch (camera_mode)
		{
		case RTS:
			KeyCallbackRTS(scan_code);
			break;
		case FREECAM:
			KeyCallbackFreeCam(scan_code);
			break;
		default:
			KeyCallbackRTS(scan_code);
			break;
		}
		if (scan_code == KeyboardLayout::ChangeCamera) {
			camera_mode = camera_mode == CameraMode::RTS ? CameraMode::FREECAM : CameraMode::RTS;
		}
	}
	void KeyCallbackRTS(int scan_code) {
		bool is_rts = true;
		_CameraMove(
			scan_code == KeyboardLayout::Left,
			scan_code == KeyboardLayout::Right,
			scan_code == KeyboardLayout::Up,
			scan_code == KeyboardLayout::Down,
			is_rts);
	};
	void KeyCallbackFreeCam(int scan_code) {
		bool is_rts = false;
		_CameraMove(
			scan_code == KeyboardLayout::Left,
			scan_code == KeyboardLayout::Right,
			scan_code == KeyboardLayout::Up,
			scan_code == KeyboardLayout::Down,
			is_rts);
	};
	void ResizeCallback(int new_width, int new_height) {
		this->width = new_width;
		this->height = new_height;
		this->projection_matrix = perspective(radians(this->FOV), (float)this->width / (float)this->height, this->near_plane, this->far_plane);
		
		is_update_need = true;
	}
	void MouseMotionCallback(float screen_x, float screen_y)
	{
		switch (camera_mode)
		{
		case RTS:
			MouseMotionCallbackRTS(screen_x, screen_y);
			break;
		case FREECAM:
			MouseMotionCallbackFreeCam(screen_x, screen_y);
			break;
		default:
			break;
		}
	}
	void MouseMotionCallbackRTS(float screen_x, float screen_y)
	{
		bool is_left = screen_x - screen_border <= 0;
		bool is_right = screen_x + 1 + screen_border >= GameSettings::SCR_WIDTH;
		bool is_down = screen_y + 1 + screen_border >= GameSettings::SCR_HEIGHT;
		bool is_up = screen_y - screen_border <= 0;
		bool is_rts = true;

		_CameraMove(is_left, is_right, is_up, is_down, is_rts, CameraSettings::mouse_motion_screen_border_velocity);
	}
	void MouseMotionCallbackFreeCam(float motion_x, float motion_y)
	{
		this->yaw_angle += (motion_x * CameraSettings::sensivity);
		this->pitch_angle -= (motion_y * CameraSettings::sensivity);
		this->pitch_angle = std::clamp(this->pitch_angle, -90.0f, 90.0f);
		if (yaw_angle > 360.0f || yaw_angle < -360.0f) {
			yaw_angle = 0.0f;
		}
		if (pitch_angle > 90.0f) {
			pitch_angle = 90.0f;
		}
		if (pitch_angle < -90.0f) {
			pitch_angle = -90.0f;
		}
		is_update_need = true;
	}

	bool IsCursorOnWindowBorder(float screen_x, float screen_y) {
		return (screen_x + 1 + screen_border >= GameSettings::SCR_WIDTH) 
			|| (screen_x - screen_border <= 0) 
			|| (screen_y + 1 + screen_border >= GameSettings::SCR_HEIGHT) 
			|| (screen_y - screen_border <= 0);
	}
private:
	void _CameraMove(bool is_left, bool is_right, bool is_up, bool is_down, bool is_rts, float velocity_delta = 1.0f) {
		if (is_right)
		{
			auto delta = right * velocity * velocity_delta;
			pos += delta;
		}
		else if (is_left)
		{
			auto delta = right * velocity * velocity_delta;
			pos -= delta;
		}
		if (is_up)
		{
			auto delta = direction * velocity * velocity_delta;
			if (is_rts) {
				delta.y = 0;
			}
			pos += delta;
		}
		else if (is_down)
		{
			auto delta = direction * velocity * velocity_delta;
			if (is_rts) {
				delta.y = 0;
			}
			pos -= delta;
		}
		is_update_need = true;
	}
};