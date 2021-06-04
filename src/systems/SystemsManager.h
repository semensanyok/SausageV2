#pragma once

#include "sausage.h"
#include "systems/Renderer.h"
#include "systems/Camera.h"
#include "systems/Gui.h"
#include "systems/BufferStorage.h"
#include "systems/Controller.h"

class SystemsManager {
public:
	Camera* camera;
	BufferStorage* buffer_storage;
	Renderer* renderer;

	SystemsManager() {};
	~SystemsManager() {};
	
	void Render() {
		renderer->Render(camera);
	}

	void InitSystems() {
		camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		renderer = new Renderer();
		renderer->InitContext();

		buffer_storage = new BufferStorage();
		buffer_storage->InitMeshBuffers();
		CheckGLError();
	}
	void Clear() {
		renderer->ClearContext();
		buffer_storage->Dispose();
		delete buffer_storage;
		delete renderer;
		delete camera;
	}
};