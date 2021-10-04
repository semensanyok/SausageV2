#pragma once

#include "sausage.h"
#include "Logging.h"
#include "Settings.h"

using namespace std;
using namespace glm;

enum CameraMode {
	RTS,
	FREE
};

class Camera
{
private:
	friend class Gui;
	friend class Renderer;
	friend class Controller;
	friend class Scene;
	friend class SystemsManager;

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
	void UpdateCameraFrontUpRight();
	void MouseWheelCallback(SDL_MouseWheelEvent& mw_event);
	void MouseWheelCallbackRTS(SDL_MouseWheelEvent& mw_event);
	void MouseWheelCallbackFreeCam(SDL_MouseWheelEvent& mw_event);
	void PreUpdate(float delta_time);
	void Update();
	void KeyCallbackRTS(int scan_code);
	void KeyCallbackFreeCam(int scan_code);
	void ResizeCallback(int new_width, int new_height);
	void MouseMotionCallback(float screen_x, float screen_y);
	void MouseMotionCallbackRTS(float screen_x, float screen_y);
	void MouseMotionCallbackFreeCam(float motion_x, float motion_y);
	void KeyCallback(int scan_code);
	bool IsCursorOnWindowBorder(float screen_x, float screen_y);
private:
	void _UpdateMatrices();
	void _CameraMove(bool is_left, bool is_right, bool is_up, bool is_down, bool is_rts, float velocity_delta = 1.0f);
};