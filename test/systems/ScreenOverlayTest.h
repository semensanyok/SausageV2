#pragma once

#include <sausage.h>
#include <ScreenOverlay.h>
#include <SystemsManager.h>
#include <gtk/gtk.h>
#include "GTKExampleApps.h"
#include <RendererContextManager.h>

class ScreenOverlayTest {
	SystemsManager* systems_manager;
  thread gtk_t;
public:
	void run() {
		Gui::enable = false;
    //GTKHelloWorld::gtk_main(0,nullptr);
    //GTKHelloWorldNonBlocking::gtk_main(0,nullptr);
    //gtk_t = thread([&] {
    //  GTKHelloWorld::gtk_main(0,nullptr);
    //  });
    //GTKHelloWorldNonBlocking::gtk_main(0, nullptr);

   //Init();
   //GTKHelloWorldOpenGl::gtk_main(0, nullptr);

    Init();
    GTKLoadedFromUIFile::gtk_main(0, nullptr, systems_manager);
	};

  	int SDL_main2() {
      gboolean done = FALSE;
    
      //Init();
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
				if (Gui::enable) {
					ImGui_ImplSDL2_ProcessEvent(&e);
				}
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
