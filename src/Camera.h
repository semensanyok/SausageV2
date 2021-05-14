#pragma once

#include "Logging.h"
#include <string>
#include "glm/gtx/string_cast.hpp"

using namespace std;
using namespace glm;

class Camera
{
public:

	Camera(float FOV, 
		float width,
		float height,
		float near_plane,
		float far_plane,
		vec3 pos,
		float yaw_angle = -90.0f,
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
		this->projection_matrix = perspective(this->FOV, this->width / this->height, this->near_plane, this->far_plane);

		UpdateCameraFrontUpRight();
		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
	};
	~Camera() {};
	void UpdateCameraFrontUpRight() {
		vec3 target = vec3(cos(radians(yaw_angle)) * cos(radians(pitch_angle)), sin(radians(pitch_angle)), sin(radians(yaw_angle)) * cos(radians(pitch_angle)));
		direction = normalize(pos - target);
		// also re-calculate the Right and Up vector
		right = normalize(cross(world_up, direction));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		up = normalize(cross(direction, right));
		_LogPos();
	}
	void KeyCallback(int scan_code, float delta_time) {
		float velocity = delta_time * movement_speed;
		vec3 new_pos = pos;
		if (scan_code == SDL_SCANCODE_W)   new_pos += direction * velocity;
		if (scan_code == SDL_SCANCODE_A)   new_pos -= right * velocity;
		if (scan_code == SDL_SCANCODE_S)   new_pos -= direction * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))
		if (scan_code == SDL_SCANCODE_D)   new_pos += right * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))

		if (!glm::isnan(new_pos.x)) {
			pos = new_pos;
		}
		this->view_matrix = lookAt(this->pos, pos + direction, this->up);

		_LogPos();
	};

	void MouseCallback(SDL_Event* e)
	{
		this->yaw_angle += ((float)e->motion.xrel * this->sensivity);
		this->pitch_angle -= ((float)e->motion.yrel * this->sensivity);
		this->pitch_angle = std::clamp(this->pitch_angle, -90.0f, 90.0f);
		if (yaw_angle > 360.0f || yaw_angle < -360.0f) {
			yaw_angle = 0.0f;
		}

		UpdateCameraFrontUpRight();
		this->view_matrix = lookAt(this->pos, pos + direction, this->up);
		_LogPos();
	}

	void _LogPos() {
		LOG("POSITION:");
		LOG(glm::to_string(pos));

		LOG("direction:");
		LOG(glm::to_string(direction));

		LOG("WORLD_UP:");
		LOG(glm::to_string(world_up));

		LOG("UP:");
		LOG(glm::to_string(up));

		LOG("RIGHT:");
		LOG(glm::to_string(right));

		LOG("YAW:");
		LOG(to_string(yaw_angle));

		LOG("PITCH:");
		LOG(to_string(pitch_angle));
	}

	mat4 view_matrix;
	mat4 projection_matrix;
	vec3 pos;
	vec3 up;
	vec3 world_up;
	vec3 right;
	vec3 direction;
	float yaw_angle;
	float pitch_angle;
	float sensivity = 1.0f;
	float movement_speed = 0.1f;

	float FOV;
	float width;
	float height;
	float near_plane;
	float far_plane;
};