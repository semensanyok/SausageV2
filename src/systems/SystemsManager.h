#pragma once

#include "sausage.h"
#include "systems/Renderer.h"
#include "systems/Camera.h"
#include "systems/Gui.h"
#include "systems/BufferStorage.h"
#include "systems/Controller.h"
#include "TextureManager.h"

class SystemsManager {
public:
	Camera* camera;
	BufferStorage* buffer;
	Renderer* renderer;
	TextureManager* texture_manager;
	FileWatcher* file_watcher;
	Samplers samplers;

	SystemsManager() {};
	~SystemsManager() {};
	
	void Render() {
		renderer->Render(camera);
	}

	void InitSystems() {
		texture_manager = new TextureManager();
		file_watcher = new FileWatcher();
		camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 3.0f, 3.0f), 0.0f, -45.0f);
		renderer = new Renderer();
		renderer->InitContext();
		InitBuffer();
		samplers = InitSamplers();
	}

	void ReloadBuffer() {
		renderer->RemoveBuffer(buffer);
		buffer->Reset();
	}

	Shader* RegisterShader(const char* vs_name, const char* fs_name) {
		auto shader = renderer->RegisterShader(vs_name, fs_name);
		function<void()> shader_reload_callback = bind(&Shader::InitOrReload, shader);
		shader_reload_callback = bind(&Renderer::AddGlCommand, renderer, shader_reload_callback);
		file_watcher->AddCallback(shader->fragment_path, shader_reload_callback);
		return shader;
	}

	void Clear() {
		DisposeBuffer();
		renderer->ClearContext();
		delete renderer;
		delete camera;
	}
private:
	void InitBuffer() {
		buffer = new BufferStorage();
		buffer->InitMeshBuffers();
		CheckGLError();
	}
	void DisposeBuffer() {
		renderer->RemoveBuffer(buffer);
		buffer->Dispose();
		delete buffer;
	}
};