#include "sausage.h"

#include "OpenGLHelpers.h"
#include "Settings.h"
#include "Shader.h"
#include "utils/AssetUtils.h"
#include "Logging.h"
#include "Texture.h"
#include "FileWatcher.h"
#include "Scene.h"
#include "systems/SystemsManager.h"

using namespace std;
using namespace glm;


SystemsManager* systems_manager;
Scene* scene;
Controller* controller;

void Init() {
	// create systems
	systems_manager = new SystemsManager();
	systems_manager->InitSystems();
	CheckGLError();
	scene = new Scene(systems_manager);
	scene->Init();
	CheckGLError();
	main_thread_id = this_thread::get_id();
}


int SDL_main(int argc, char** argv)
{
	Init();
	systems_manager->async_manager->Run();
	while (!GameSettings::quit) {
		systems_manager->UpdateDeltaTime();
		
#ifdef SAUSAGE_PROFILE_ENABLE
		auto proft1 = chrono::steady_clock::now();
#endif
		scene->PrepareDraws();
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::prepare_draws_ns = chrono::steady_clock::now() - proft1;
#endif

		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ImGui_ImplSDL2_ProcessEvent(&e);
			systems_manager->controller->ProcessEvent(&e);
		}
		systems_manager->Render();

#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::total_frame_ns = chrono::steady_clock::now() - proft1;
#endif
		CheckGLError();
		GameSettings::milliseconds_since_start = SDL_GetTicks();
	}
	systems_manager->Clear();
	delete systems_manager;
	return 0;
}
