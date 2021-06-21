﻿#include "sausage.h"

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
	controller = new Controller(systems_manager->camera);
	CheckGLError();
}

int SDL_main(int argc, char** argv)
{
	auto log_thread = LogIO(quit);
	
	Init();
	systems_manager->file_watcher->Start(quit);
	
	while (!quit) {
		systems_manager->UpdateDeltaTime();
		systems_manager->physics_manager->Simulate(systems_manager->delta_time);
		systems_manager->physics_manager->UpdateTransforms();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			controller->ProcessEvent(&e, systems_manager->delta_time, quit);
		}
		systems_manager->Render();
	
		//this_thread::sleep_for(std::chrono::milliseconds(1000));
		CheckGLError();
	}
	systems_manager->Clear();
	
	log_thread.join();
	systems_manager->file_watcher->Join();
	return 0;
}
