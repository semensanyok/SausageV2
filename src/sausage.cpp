#include "sausage.h"

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "Settings.h"
#include "Mesh.h"
#include "Settings.h"
#include "Shader.h"
#include "AssetUtils.h"
#include "Logging.h"
#include "Texture.h"
#include "systems/Renderer.h"
#include "systems/Camera.h"
#include "systems/Gui.h"
#include "systems/BufferStorage.h"

using namespace std;
using namespace glm;

// Systems
Camera* camera;
BufferStorage* buffer_storage;
Renderer* renderer;

// Game state
Samplers samplers;
Shader shader;
float delta_time = 0;
float last_ticks = 0;
bool quit = false;

void ProcessEvent(SDL_Event* e)
{
	switch (e->type) {
	case SDL_MOUSEMOTION:
		// camera->MouseMotionCallback(e);
		break;
	case SDL_MOUSEBUTTONDOWN:
		break;
	case SDL_MOUSEBUTTONUP:
		break;
	case SDL_MOUSEWHEEL:
		break;
	case SDL_QUIT:
		quit = 1;
		break;

	case SDL_WINDOWEVENT:
		switch (e->window.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SCR_HEIGHT = e->window.data1;
			SCR_WIDTH = e->window.data2;
			break;
		}
		break;

	case SDL_KEYDOWN: {
		int k = e->key.keysym.sym;
		int s = e->key.keysym.scancode;

		// Intercept SHIFT + ~ key stroke to toggle libRocket's 
					// visual debugger tool
		if (e->key.keysym.sym == SDLK_BACKQUOTE &&
			e->key.keysym.mod == KMOD_LSHIFT)
		{
			break;
		}

		SDL_Keymod mod;
		mod = SDL_GetModState();
		if (k == SDLK_ESCAPE)
			quit = 1;
		camera->KeyCallbackRTS(s, delta_time);
		break;
	}
	case SDL_KEYUP: {
		break;
	}
	}
}
void InitGame() {
	// create systems
	camera = new Camera(80.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 0.0f, 3.0f), 0.0f, -45.0f);
	renderer = new Renderer();
	buffer_storage = new BufferStorage();

	// init
	renderer->InitContext();
	buffer_storage->InitMeshVAO();
	buffer_storage->InitMeshBuffers();
	samplers = InitSamplers();
	InitGuiContext(renderer->window, renderer->context);

	vector<vector<Vertex>> vertices;
	vector<vector<unsigned int>> indices;
	vector<unsigned int> draw_ids;

	LoadMeshes(vertices, indices, draw_ids, GetModelPath("cube.fbx"));
	Texture* tex = LoadTextureArray("Image0001.png", "Image0000_normal.png", "Image0000_specular.png", "Image0000_height.png", buffer_storage);
	tex->MakeResident();
	vector<unsigned int> texture_ids = { (unsigned int)(tex->texture_handle_ARB) };

	// load render data
	buffer_storage->BufferMeshData(vertices, indices, draw_ids, texture_ids, nullptr);
	vector<mat4> transforms{ mat4(1) };
	buffer_storage->BufferTransforms(transforms, nullptr);

	shader = Shader(GetShaderPath("bindless_vs.glsl"), GetShaderPath("bindless_fs.glsl"));
	shader.setMat4("projection_view", camera->projection_matrix * camera->view_matrix);
	glUseProgram(shader.id);
}

void _UpdateDeltaTime() {
	float this_ticks = SDL_GetTicks();
	delta_time = this_ticks - last_ticks;
	last_ticks = this_ticks;
}

void Render() {
	
	glEnable(GL_DEPTH_TEST);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer_storage->transform_buffer);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, buffer_storage->texture_id_for_draw_id_buffer);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buffer_storage->command_buffer);
	glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, buffer_storage->command_total, 0);
}

int SDL_main(int argc, char** argv)
{
	auto log_thread = LogIO(quit);

	InitGame();

	while (!quit) {
		_UpdateDeltaTime();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			ProcessEvent(&e);
		}
		Render();
		RenderGui(renderer->window, camera);
		// Draw
		SDL_GL_SwapWindow(renderer->window);
	}
	CleanupGui();
	renderer->ClearContext();
	log_thread.join();
	return 0;
}
