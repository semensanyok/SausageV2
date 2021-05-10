#pragma once

#include "TestUtils.h"
#include <string>
#include "glm/gtx/string_cast.hpp"

using namespace std;

class Camera
{
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
		UpdateCameraFrontUpRight();

		this->projection_matrix = perspective(this->FOV, this->width / this->height, this->near_plane, this->far_plane);

		this->view_matrix = lookAt(this->pos, this->pos + this->front, this->up);
	};
	~Camera() {};
	void UpdateCameraFrontUpRight() {
		this->front.x = cos(radians(pitch_angle)) * sin(radians(yaw_angle));
		this->front.y = sin(radians(pitch_angle));
		this->front.z = sin(radians(pitch_angle)) * cos(radians(yaw_angle));
		//this->front = normalize(this->front);
		// also re-calculate the Right and Up vector
		this->right = normalize(cross(this->front, this->up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		//this->up = normalize(cross(this->right, this->world_up));
		_LogPos();
	}
	void KeyCallback(int scan_code, float delta_time) {
		float velocity = delta_time * movement_speed;
		vec3 new_pos = pos;
		if (scan_code == SDL_SCANCODE_W)   new_pos += front * velocity;
		if (scan_code == SDL_SCANCODE_A)   new_pos -= right * velocity;
		if (scan_code == SDL_SCANCODE_S)   new_pos -= front * velocity;
		if (scan_code == SDL_SCANCODE_D)   new_pos += right * velocity;

		if (!glm::isnan(new_pos.x)) {
			pos = new_pos;
		}
		this->view_matrix = lookAt(this->pos, this->pos + this->front, this->up);

		_LogPos();
	};

	void MouseCallback(SDL_Event* e)
	{
		this->yaw_angle += ((float)e->motion.xrel * this->sensivity);
		this->pitch_angle += ((float)e->motion.yrel * this->sensivity);
		this->yaw_angle = std::clamp(this->yaw_angle, -90.0f, 90.0f);
		this->pitch_angle = std::clamp(this->pitch_angle, -90.0f, 90.0f);
		
		UpdateCameraFrontUpRight();
		this->view_matrix = lookAt(this->pos, this->pos + this->front, this->up);
	}

	void _LogPos() {
		LOG("POSITION:");
		LOG(glm::to_string(pos));

		LOG("FRONT:");
		LOG(glm::to_string(front));

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
	vec3 front;
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