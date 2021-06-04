#pragma once

#include "sausage.h"
#include "Logging.h"

using namespace std;
using namespace glm;

class Camera
{
private:
	void _UpdateMatrices() {
		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
		this->projection_view = projection_matrix * view_matrix;
	}
public:

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
	void MouseWheelCallbackRTS(SDL_MouseWheelEvent& mw_event, float delta_time) {
		this->pos.y -= (mw_event.y * scroll_speed);
		
		_UpdateMatrices();
	};
	void KeyCallbackRTS(int scan_code, float delta_time) {
		float velocity = delta_time * movement_speed;
		if (scan_code == SDL_SCANCODE_W)   pos -= vec3(0, 0, velocity);
		if (scan_code == SDL_SCANCODE_A)   pos -= right * velocity;
		if (scan_code == SDL_SCANCODE_S)   pos += vec3(0, 0, velocity);
		if (scan_code == SDL_SCANCODE_D)   pos += right * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))

		_UpdateMatrices();
	};
	void ResizeCallback(int new_width, int new_height) {
		this->width = new_width;
		this->height = new_height;
		this->projection_matrix = perspective(radians(this->FOV), (float)this->width / (float)this->height, this->near_plane, this->far_plane);
		this->projection_view = projection_matrix * view_matrix;

		_UpdateMatrices();
	}
	void MouseMotionCallbackRTS(SDL_Event* e)
	{
		this->yaw_angle += ((float)e->motion.xrel * this->sensivity);
		this->pitch_angle -= ((float)e->motion.yrel * this->sensivity);
		if (this->pitch_angle < -90.0f) {
			this->pitch_angle = -90.0f;
		}
		if (this->pitch_angle > 90.0f) {
			this->pitch_angle = 90.0f;
		}
		// this->pitch_angle = std::clamp(this->pitch_angle, -90.0f, 90.0f);
		if (yaw_angle > 360.0f || yaw_angle < -360.0f) {
			yaw_angle = 0.0f;
		}

		UpdateCameraFrontUpRight();
		_UpdateMatrices();
	}

	void KeyCallbackFreeCam(int scan_code, float delta_time) {
		float velocity = delta_time * movement_speed;
		if (scan_code == SDL_SCANCODE_W)   pos += direction * velocity;
		if (scan_code == SDL_SCANCODE_A)   pos -= right * velocity;
		if (scan_code == SDL_SCANCODE_S)   pos -= direction * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))
		if (scan_code == SDL_SCANCODE_D)   pos += right * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))

		_UpdateMatrices();
	};

	void MouseMotionCallbackFreeCam(SDL_Event* e)
	{
		this->yaw_angle += ((float)e->motion.xrel * this->sensivity);
		this->pitch_angle -= ((float)e->motion.yrel * this->sensivity);
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

		UpdateCameraFrontUpRight();
		_UpdateMatrices();
	}

	mat4 view_matrix;
	mat4 projection_matrix;
	mat4 projection_view;
	vec3 pos;
	vec3 up;
	vec3 world_up;
	vec3 right;
	vec3 direction;
	float yaw_angle;
	float pitch_angle;
	float sensivity = 0.1f;
	float movement_speed = 0.1f;
	float scroll_speed = 1.0f;

	float FOV;
	float width;
	float height;
	float near_plane;
	float far_plane;
};