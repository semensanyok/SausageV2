#pragma once

#include <assert.h>
#include <sausage.h>
#include <ScreenOverlayManager.h>

using namespace std;

class ScreenOverlayManagerTest : public ScreenOverlayManager {
public:
	void run() {
    GameSettings::SCR_WIDTH = 30;
    GameSettings::SCR_HEIGHT = 30;
    Init();
    assert(cell_height == 10);
    assert(total_cells_x == 3);
    assert(total_cells_y == 3);
    Point expected_positions[] = {
      {0.0, 0.0},{0.0, 10.0},{0.0, 20.0},
      {10.0, 0.0},{10.0, 10.0},{10.0, 20.0},
      {20.0, 0.0},{20.0, 10.0},{20.0, 20.0},
    };
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        cout << expected_positions[i * 3 + j] << "==" << all_cells[i * 3 + j].pos << endl;
        assert(expected_positions[i * 3 + j] == all_cells[i * 3 + j].pos);
      }
    }
	};
};
