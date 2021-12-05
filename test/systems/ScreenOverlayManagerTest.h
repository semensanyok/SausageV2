#pragma once

#include <assert.h>
#include <sausage.h>
#include <SystemsManager.h>
#include <ScreenOverlayManager.h>
#include "SausageTestBase.h"

using namespace std;

class ScreenOverlayManagerTest : public SausageTestBase {
  bool is_inited = false;
  SystemsManager* systems_manager;

  string test_string = "test_string";
public:
	void run() {
    TestInit();
    Test_AddInteractiveElement();
	};

  void TestInit() {
    GameSettings::SCR_WIDTH = 30;
    GameSettings::SCR_HEIGHT = 30;
    InitSystemsManager();
    assert(systems_manager->screen_overlay_manager->cell_height == 10);
    assert(systems_manager->screen_overlay_manager->total_cells_x == 3);
    assert(systems_manager->screen_overlay_manager->total_cells_y == 3);
    Point expected_positions[] = {
      {0.0, 0.0},{0.0, 10.0},{0.0, 20.0},
      {10.0, 0.0},{10.0, 10.0},{10.0, 20.0},
      {20.0, 0.0},{20.0, 10.0},{20.0, 20.0},
    };
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        cout << expected_positions[i * 3 + j] << "==" << systems_manager->screen_overlay_manager->all_cells[i * 3 + j].pos << endl;
        assert(expected_positions[i * 3 + j] == systems_manager->screen_overlay_manager->all_cells[i * 3 + j].pos);
      }
    }
    is_inited = true;
  }

  void Test_AddInteractiveElement() {
    assert(is_inited);
    auto batch_mesh = systems_manager->screen_overlay_manager->GetTextMesh(test_string, 0.0, 0.0);
    auto bp_raw = batch_mesh.first.get();
    auto mesh = batch_mesh.second;
    //systems_manager->screen_overlay_manager->SubmitDraw(bp_raw, mesh);
    systems_manager->screen_overlay_manager->AddInteractiveElement(mesh,bp_raw->x_max,bp_raw->x_min,bp_raw->y_max,bp_raw->y_min);
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
  void InitSystemsManager() {
		systems_manager = new SystemsManager();
		systems_manager->InitSystems();
		CheckGLError();
	}
};
