#pragma once

#include "sausage.h"
#include "Logging.h"

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
		this->projection_matrix = perspective(this->FOV, this->width / this->height, this->near_plane, this->far_plane);

		UpdateCameraFrontUpRight();
		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
	};
	~Camera() {};
	void UpdateCameraFrontUpRight() {
		vec3 target = vec3(sin(radians(yaw_angle)) * cos(radians(pitch_angle)), sin(radians(pitch_angle)), -cos(radians(yaw_angle)) * cos(radians(pitch_angle)));
		direction = normalize(target);
		right = normalize(cross(target, world_up));
		up = normalize(cross(right, target));
	}
	void KeyCallbackRTS(int scan_code, float delta_time) {
		float velocity = delta_time * movement_speed;
		if (scan_code == SDL_SCANCODE_W)   pos += vec3(0, velocity, 0);
		if (scan_code == SDL_SCANCODE_A)   pos += right * velocity;
		if (scan_code == SDL_SCANCODE_S)   pos -= vec3(0, velocity, 0); // glm::normalize(glm::cross(cameraFront, cameraUp))
		if (scan_code == SDL_SCANCODE_D)   pos -= right * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))

		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
	};

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
		this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
	}

	//void KeyCallbackFreeCam(int scan_code, float delta_time) {
	//	float velocity = delta_time * movement_speed;
	//	if (scan_code == SDL_SCANCODE_W)   pos += direction * velocity;
	//	if (scan_code == SDL_SCANCODE_A)   pos += right * velocity;
	//	if (scan_code == SDL_SCANCODE_S)   pos -= direction * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))
	//	if (scan_code == SDL_SCANCODE_D)   pos -= right * velocity; // glm::normalize(glm::cross(cameraFront, cameraUp))

	//	this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);

	//	_LogPos();
	//};

	//void MouseMotionCallbackFreeCam(SDL_Event* e)
	//{
	//	this->yaw_angle += ((float)e->motion.xrel * this->sensivity);
	//	this->pitch_angle -= ((float)e->motion.yrel * this->sensivity);
	//	this->pitch_angle = std::clamp(this->pitch_angle, -90.0f, 90.0f);
	//	if (yaw_angle > 360.0f || yaw_angle < -360.0f) {
	//		yaw_angle = 0.0f;
	//	}

	//	UpdateCameraFrontUpRight();
	//	this->view_matrix = lookAt(this->pos, this->pos + this->direction, this->up);
	//	_LogPos();
	//}

	mat4 view_matrix;
	mat4 projection_matrix;
	vec3 pos;
	vec3 up;
	vec3 world_up;
	vec3 right;
	vec3 direction;
	float yaw_angle;
	float pitch_angle;
	float sensivity = 0.1f;
	float movement_speed = 0.1f;

	float FOV;
	float width;
	float height;
	float near_plane;
	float far_plane;
};