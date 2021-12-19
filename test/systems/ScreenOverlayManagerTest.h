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
    //TestInit();
    //Test_AddInteractiveElement();
    Gui::enable = false;
    Test_InitPauseMenu();
    this->SDL_main2();
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

  void Test_InitPauseMenu() {
    GameSettings::SCR_WIDTH = 800;
    GameSettings::SCR_HEIGHT = 800;
		Init();
    int button_font_size = FontSizes::STANDART;
    int back_indent = 5;
    vec3 text_color = {255,0,0};
    vec3 back_color = {0,0,0};
    PauseMenuSettings sets = {button_font_size,back_indent,text_color,back_color};

    som->InitPauseMenu(sets);
    som->ActivatePauseMenu();
  }

  void Test_AddInteractiveElement() {
    assert(is_inited);
    auto batch_back = som->_GetBackgroundMesh({255,0,0}, 0, 0, 13, 13);
    auto batch_mesh = som->_GetTextMesh(test_string, {0,255,0}, 0.0, 0.0, test_font_size);
    auto mesh = batch_mesh.second;
    auto mesh_data = batch_mesh.first.get();
    auto back = batch_back.second;
    auto back_data = batch_back.first.get();

    auto NODE_0_OPEN_ORDER = 0;
    //som->SubmitDraw(bp_raw, mesh);
    auto node1 = som->_AddUINode(mesh,back,back_data->x_max,back_data->x_min,
      back_data->y_max,back_data->y_min,AnchorRelativeToNodePosition::LeftBottom, NODE_0_OPEN_ORDER);
    // expected overlap bottom left corner of screen
    set<int> expected_overlap_cells_ids = {0,1,som->total_cells_x,som->total_cells_x + 1};
    const int expected_cells_checked = 4;
    int cells_checked = 0;
    for (int i = 0; i < som->total_cells_x * som->total_cells_y;i++) {
      if (expected_overlap_cells_ids.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 1);
        assert(node1 == (*(som->all_cells[i].nodes.begin())));
        cells_checked++;
      }
    }
    assert(cells_checked == expected_cells_checked);

    // TODO: fix SET removes nodes instead of adding if open_order is same.
    auto NODE_1_OPEN_ORDER = 1;
    // test new text written above
    auto horizontal_offet = som->cell_width;
    auto batch_back2 = som->_GetBackgroundMesh({255,0,0}, 0.0, horizontal_offet, 13, 13);
    auto batch_mesh2 = som->_GetTextMesh(test_string, {0,255,0}, 0.0, horizontal_offet, test_font_size);
    auto mesh2 = batch_mesh2.second;
    auto mesh_data2 = batch_mesh2.first.get();
    auto back2 = batch_back2.second;
    auto back_data2 = batch_back2.first.get();
    auto node2 = som->_AddUINode(mesh2,back2,back_data2->x_max,back_data2->x_min,
      back_data2->y_max,back_data2->y_min,AnchorRelativeToNodePosition::LeftBottom, NODE_1_OPEN_ORDER);

    expected_overlap_cells_ids = {1,2,som->total_cells_x + 1,som->total_cells_x + 2};
    set<int> expected_overlap_cells_ids_with_previous = {1,som->total_cells_x + 1};
    cells_checked = 0;
    for (int i = 0; i < som->total_cells_x * som->total_cells_y;i++) {
      cout << i << " : " << som->all_cells[i].nodes.size() << endl;
    }
    for (int i = 0; i < som->total_cells_x * som->total_cells_y;i++) {
      if (expected_overlap_cells_ids_with_previous.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 2);
        assert(node2 == (*(som->all_cells[i].nodes.begin())));
        cells_checked++;
      } else if (expected_overlap_cells_ids.contains(i)) {
        assert(som->all_cells[i].nodes.size() == 1);
        assert(node2 == (*(som->all_cells[i].nodes.begin())));
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
