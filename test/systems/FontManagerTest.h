#pragma once

#include <assert.h>
#include <sausage.h>
#include <AssetUtils.h>
#include <SystemsManager.h>
#include <FontManager.h>

using namespace std;
using namespace glm;

class FontManagerTest {
	SystemsManager* systems_manager;
	
	string teststr1 = "qwe";
public:
	void run() {
		SDL_main2();
	};
	void SubmitTextDraw() {
		systems_manager->font_manager->WriteTextUI(teststr1, 0, 0);
	}
	int SDL_main2()
	{
		Init();

		systems_manager->async_manager->Run();
		while (!GameSettings::quit) {
			systems_manager->PreUpdate();

#ifdef SAUSAGE_PROFILE_ENABLE
			auto proft1 = chrono::steady_clock::now();
#endif
			//scene->PrepareDraws();
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
	void Init() {
		systems_manager = new SystemsManager();
		systems_manager->InitSystems();
		CheckGLError();
	}
};