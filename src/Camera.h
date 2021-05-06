#pragma once

class Camera
{
public:

	Camera(float FOV, float width, float height, float near_plane, float far_plane, vec3 pos, float yaw_angle = 0.0f, float pitch_angle = 0.0f, vec3 up = vec3(0.0f, 1.0f, 0.0f), vec3 world_up = vec3(0.0f, 1.0f, 0.0f)) {
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

		this->view_matrix = lookAt(this->pos, this->front, this->up);
	};
	~Camera() {};
	void Rotate(double yaw_angle, double pitch_angle) {
		this->yaw_angle = std::clamp(this->yaw_angle += (yaw_angle * this->sensivity), -90.0f, 90.0f);
		this->pitch_angle = std::clamp(this->pitch_angle += (pitch_angle * this->sensivity), -90.0f, 90.0f);
		UpdateCameraFrontUpRight();
		this->view_matrix = lookAt(this->pos, this->pos + this->front, this->up);
	}
	void UpdateCameraFrontUpRight() {
		this->front.x = cos(radians(this->yaw_angle)) * cos(radians(this->pitch_angle));
		this->front.y = sin(radians(this->pitch_angle));
		this->front.z = sin(radians(this->yaw_angle)) * cos(radians(this->pitch_angle));
		this->front = normalize(this->front);
		// also re-calculate the Right and Up vector
		this->right = normalize(cross(this->front, this->up));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		this->up = normalize(cross(this->right, this->world_up));
	}
	//void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	//	float velocity = delta_time * movement_speed;
	//	InputEvent input_event = KeyMapping::GetInstance().GetInputKey(key);
	//	switch (input_event)
	//	{
	//	case InputEvent::UP:
	//		pos += front * velocity;
	//		break;
	//	case InputEvent::DOWN:
	//		pos -= front * velocity;
	//		break;
	//	case InputEvent::LEFT:
	//		pos -= right * velocity;
	//		break;
	//	case InputEvent::RIGHT:
	//		pos += right * velocity;
	//		break;
	//	case InputEvent::NONE:
	//		break;
	//	default:
	//		break;
	//	}
	//	this->view_matrix = lookAt(this->pos, this->pos + this->front, this->up);
	//};
	void MouseCallback(SDL_Event* e)
	{
		float yaw_angle = (float)e->motion.xrel;
		float pitch_angle = (float)e->motion.yrel;

		this->yaw_angle = std::clamp(this->yaw_angle += (yaw_angle * this->sensivity), -90.0f, 90.0f);
		this->pitch_angle = std::clamp(this->pitch_angle += (pitch_angle * this->sensivity), -90.0f, 90.0f);
		// (float)e->motion.xrel / width, (float)e->motion.yrel / height;
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
	float sensivity = 0.1f;
	float movement_speed = 0.1f;

	float FOV;
	float width;
	float height;
	float near_plane;
	float far_plane;
};