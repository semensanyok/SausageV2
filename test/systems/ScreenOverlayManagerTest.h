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
  ScreenOverlayManager* som;

  string test_string = "te";
  int test_font_size = 15;
public:
	void run() {
    TestInit();
    Test_AddInteractiveElement();
	};

  void TestInit() {
    GameSettings::SCR_WIDTH = 30;
    GameSettings::SCR_HEIGHT = 30;
		Init();
    assert(som->cell_height == 10);
    assert(som->total_cells_x == 3);
    assert(som->total_cells_y == 3);

    Point expected_positions[] = {
      {0.0, 0.0},{0.0, 10.0},{0.0, 20.0},
      {10.0, 0.0},{10.0, 10.0},{10.0, 20.0},
      {20.0, 0.0},{20.0, 10.0},{20.0, 20.0},
    };
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        cout << expected_positions[i * 3 + j] << "==" << som->all_cells[i * 3 + j].pos << endl;
        assert(expected_positions[i * 3 + j] == som->all_cells[i * 3 + j].pos);
      }
    }
    systems_manager->font_manager->_InitFontTexture(test_font_size);
    is_inited = true;
  }

  void Test_AddInteractiveElement() {
    assert(is_inited);
    auto batch_mesh = som->GetTextMesh(test_string, 0.0, 0.0, test_font_size);
    auto bp_raw = batch_mesh.first.get();
    auto mesh = batch_mesh.second;
    //som->SubmitDraw(bp_raw, mesh);
    som->AddInteractiveElement(mesh,bp_raw->x_max,bp_raw->x_min,bp_raw->y_max,bp_raw->y_min,AnchorRelativeToNodePosition::LeftBottom);
    // expected overlap bottom left corner of screen
    set<int> expected_overlap_cells_ids = {0,1,som->total_cells_x,som->total_cells_x + 1};
    const int expected_cells_checked = 4;
    int cells_checked = 0;
    for (int i = 0; i < som->total_cells_x * som->total_cells_y;i++) {
      if (expected_overlap_cells_ids.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 1);
        cells_checked++;
      }
    }
    assert(cells_checked == expected_cells_checked);


    // test new text written above
    auto x_offet = som->cell_width;
    auto batch_mesh2 = som->GetTextMesh(test_string, x_offet, 0.0, test_font_size);
    auto bp_raw2 = batch_mesh2.first.get();
    auto mesh2 = batch_mesh2.second;
    som->AddInteractiveElement(mesh2,bp_raw2->x_max,bp_raw2->x_min,bp_raw2->y_max,bp_raw2->y_min,AnchorRelativeToNodePosition::LeftBottom);
    set<int> expected_overlap_cells_ids_with_previous = {som->total_cells_x,som->total_cells_x + 1};
    cells_checked = 0;
    for (int i = 0; i < som->total_cells_x * som->total_cells_y;i++) {
      if (expected_overlap_cells_ids_with_previous.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 2);
        cells_checked++;
      } else if (expected_overlap_cells_ids.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 1);
        cells_checked++;
      }
    }
    assert(cells_checked == expected_cells_checked);

  }

  int SDL_main2()
	{
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
    som = systems_manager->screen_overlay_manager;
		CheckGLError();
	}
};
