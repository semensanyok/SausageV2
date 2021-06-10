#include "sausage.h"

#include "sausage.h"

#include "Settings.h"
#include "Shader.h"
#include "utils/AssetUtils.h"
#include "Logging.h"
#include "Texture.h"
#include "systems/SystemsManager.h"
#include "OpenGLHelpers.h"
#include "Scene.h"

using namespace std;
using namespace glm;

SystemsManager* systems_manager;
Scene* scene;
Controller* controller;

// Game state
Samplers samplers;
float delta_time = 0;
float last_ticks = 0;
bool quit = false;


void Init() {
	// create systems
	systems_manager = new SystemsManager();
	systems_manager->InitSystems();
	CheckGLError();
	samplers = InitSamplers();
	scene = new Scene(systems_manager->buffer_storage, systems_manager->texture_manager, systems_manager->camera, systems_manager->renderer, samplers);
	scene->Init();
	scene->PrepareDraws();
	controller = new Controller(systems_manager->camera, scene);
	CheckGLError();
}

void _UpdateDeltaTime() {
	float this_ticks = SDL_GetTicks();
	delta_time = this_ticks - last_ticks;
	last_ticks = this_ticks;
}

int SDL_main(int argc, char** argv)
{
	auto log_thread = LogIO(quit);

	Init();

	while (!quit) {
		_UpdateDeltaTime();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			controller->ProcessEvent(&e, delta_time, quit);
		}
		systems_manager->Render();
	}
	systems_manager->Clear();

	log_thread.join();
	return 0;
}
