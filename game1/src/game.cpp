#include "game.h"
#include "Scene.h"

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
}


int SDL_main(int argc, char** argv)
{
	Init();
	systems_manager->async_manager->Run();
	while (!GameSettings::quit) {
		systems_manager->PreUpdate();

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
		systems_manager->Update();
#ifdef SAUSAGE_PROFILE_ENABLE
		ProfTime::total_frame_ns = chrono::steady_clock::now() - proft1;
#endif
		CheckGLError();
	}
	systems_manager->Clear();
	delete systems_manager;
	return 0;
}