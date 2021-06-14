#include "sausage.h"

#include "sausage.h"

#include "Settings.h"
#include "Shader.h"
#include "utils/AssetUtils.h"
#include "Logging.h"
#include "Texture.h"
#include "OpenGLHelpers.h"
#include "FileWatcher.h"
#include "Scene.h"
#include "systems/SystemsManager.h"

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
	scene = new Scene(systems_manager);
	scene->Init();
	scene->PrepareDraws();
	controller = new Controller(systems_manager->camera);
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
	systems_manager->file_watcher->Start(quit);

	while (!quit) {
		_UpdateDeltaTime();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			controller->ProcessEvent(&e, delta_time, quit);
		}
		systems_manager->Render();
		CheckGLError();
	}
	systems_manager->Clear();

	log_thread.join();
	systems_manager->file_watcher->Join();
	return 0;
}
