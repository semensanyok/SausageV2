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
bool quit = false;


void Init() {
	// create systems
	systems_manager = new SystemsManager();
	systems_manager->InitSystems();
	CheckGLError();
	scene = new Scene(systems_manager);
	scene->Init();
	scene->PrepareDraws();
	CheckGLError();
}

int SDL_main(int argc, char** argv)
{
	auto log_thread = LogIO(quit);
	
	Init();
	systems_manager->file_watcher->Start(quit);
	
	while (!quit) {
		systems_manager->UpdateDeltaTime();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			systems_manager->controller->ProcessEvent(&e, systems_manager->delta_time, quit);
		}
		systems_manager->Render();
	
		systems_manager->physics_manager->Simulate(systems_manager->delta_time);
		systems_manager->physics_manager->UpdateTransforms();		
		CheckGLError();
	}
	systems_manager->Clear();
	
	log_thread.join();
	systems_manager->file_watcher->Join();
	delete systems_manager;
	return 0;
}
