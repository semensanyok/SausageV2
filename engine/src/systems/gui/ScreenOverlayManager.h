#pragma once

#include "sausage.h"
#include "Structures.h"

/*
Screen splitted to equaly sized cells;
Each cell holds top/bottom/left/right neighbors;
Each cell contains sorted set of UI nodes;

UI node:
- has size. overlap many cells;
- start cell == cell of anchor point. to remove node from all cells on destroy.
- open_order. draw and OnClick priority.
              can have parent. (drop-down menu).
              open_order=parent ? parent.open_order + 1 : global_open_order + 1.
Cell contains 
*/

class UINode;

struct Point {
  int x;
  int y;
};
bool operator==(const Point& p1, const Point& p2);
template <>
struct std::formatter<Point> : std::formatter<std::string> {
  auto format(Point p, format_context& ctx) {
    return formatter<string>::format(
      std::format("(x={}, y={})", p.x, p.y), ctx);
  }
};
std::ostream& operator<<(std::ostream& in, const Point& pt);
class ScreenCell
{
public:
  Point pos;
  ScreenCell* top;
  ScreenCell* bottom;
  ScreenCell* left;
  ScreenCell* right;
  set<UINode> nodes;
};

class ScreenOverlayManager
{
protected:
  int init_screen_width, init_screen_height;
  int cell_width = 10, cell_height = 10;
  int total_cells_x, total_cells_y;
  ScreenCell* all_cells;
public:
  ScreenOverlayManager() {};
  void Init() {
    init_screen_width = GameSettings::SCR_WIDTH;
    init_screen_height = GameSettings::SCR_HEIGHT;
    total_cells_x = GameSettings::SCR_WIDTH/cell_width;
    total_cells_y = GameSettings::SCR_HEIGHT/cell_height;

    all_cells = new ScreenCell[total_cells_x * total_cells_y];
    for (int w = 0; w < total_cells_x; w++) {
      for (int h = 0; h < total_cells_y; h++)  {
        ScreenCell& cell = all_cells[w * total_cells_x + h];
        cell.pos.x = w * cell_width;
        cell.pos.y = h * cell_height;
        if (w != total_cells_x - 1) {
          cell.right = &all_cells[(w + 1) * total_cells_x + h];
        }
        if (h != total_cells_y - 1) {
          cell.top = &all_cells[w * total_cells_x + (h + 1)];
        }
        if (w != 0) {
          cell.left = &all_cells[(w - 1) * total_cells_x + h];
        }
        if (h != 0) {
          cell.bottom = &all_cells[w * total_cells_x + (h - 1)];
        }
      }
    }
  }
  void OnResize() {
    float height_delta = (float)GameSettings::SCR_HEIGHT / init_screen_height;
    float width_delta = (float)GameSettings::SCR_WIDTH / init_screen_width;
    init_screen_width = GameSettings::SCR_WIDTH;
    init_screen_height = GameSettings::SCR_HEIGHT;
    cell_width = cell_width * width_delta;
    cell_height = cell_height * height_delta;
    // TODO: pass scale to shader
    //for (int w = 0; w < total_cells_x; w++) {
    //  for (int h = 0; h < total_cells_y; h++)  {
    //    auto& cell = all_cells[w][h];
    //    cell.pos.x *= width_delta;
    //    cell.pos.y *= height_delta;
    //}
  }
};

enum AnchorRelativeToNodePosition {
  LeftBottom,
  LeftTop,
  RightBottom,
  RightTop,
};

class UINode
{
  Point anchor_position;
  AnchorRelativeToNodePosition anchor_relative_to_node_position;
  int width;
  int height;
  ScreenCell start_cell;
  // last opened drawn last, above previously opened windows.
  int open_order;
public:
  void Call() {};
  void OnHover() {};
  void OnPressed() {};
  void OnReleased() {};
  void OnDestroy() {};
  // opened last > opened first.
  bool operator< (const UINode &other) {
    return open_order < other.open_order;
  };
};
