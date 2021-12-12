#pragma once

#include "sausage.h"
#include "Structures.h"
#include "ControllerUtils.h"
#include "UIBufferConsumer.h"
#include "FontManager.h"

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
*/
using namespace std;

class UINode;
class ScreenCell;

class UINode
{
  friend class ScreenOverlayManager;
  UINodePosition node_position;
  ScreenCell* start_cell;
  // last opened drawn last, above previously opened windows.
  vector<function<void()>> on_destroys;
public:
  int open_order;
  void Call() { cout << "Call" << endl; };
  void OnHover() { cout << "OnHover" << endl; };
  void OnPressed() { cout << "OnPressed" << endl; };
  void OnReleased() { cout << "OnReleased" << endl; };
  void OnDestroy() {};
private:
  UINode(UINodePosition node_position, ScreenCell* start_cell, int open_order) :
    node_position{node_position},
    start_cell{start_cell},
    open_order {open_order} {}
};

class ScreenCell
{
public:
  Point pos;
  ScreenCell* top;
  ScreenCell* bottom;
  ScreenCell* left;
  ScreenCell* right;
  set<UINode*, decltype([](UINode* lhs, UINode* rhs) {
        return lhs->open_order > rhs->open_order;
})> nodes;
};

class ScreenOverlayManager
{
  friend class SausageTestBase;
  friend class ScreenOverlayManagerTest;
private:
  int init_screen_width, init_screen_height;
  int cell_width = 10, cell_height = 10;
  int total_cells_x, total_cells_y;
  ScreenCell* all_cells;

  vector<MeshDataUI*> drawn_ui_elements;
  vector<UINode*> interactive_ui_elements;
  UIBufferConsumer* buffer;
  MeshManager* mesh_manager;
  FontManager* font_manager;
  DrawCall* draw_call_ui;
  Renderer* renderer;
  const unsigned int command_buffer_size = 1;

public:
  ScreenOverlayManager(
    UIBufferConsumer* buffer,
    Shaders* shaders,
    MeshManager* mesh_manager,
    FontManager* font_manager,
    Renderer* renderer) :
    buffer{buffer},
    mesh_manager{mesh_manager},
    font_manager{font_manager},
    renderer {renderer} {
    draw_call_ui = new DrawCall();
    draw_call_ui->shader = shaders->font_ui;
    draw_call_ui->mode = GL_TRIANGLES;
    draw_call_ui->buffer = (BufferConsumer*)buffer;
    draw_call_ui->command_buffer =
        draw_call_ui->buffer->CreateCommandBuffer(command_buffer_size);
  };
  void Init() {
    _InitScreenLayout();
    renderer->AddDraw(draw_call_ui, DrawOrder::UI);
    draw_call_ui->buffer->ActivateCommandBuffer(draw_call_ui->command_buffer);
  }
  void Deactivate() {
    renderer->RemoveDraw(draw_call_ui, DrawOrder::UI);
    draw_call_ui->buffer->RemoveCommandBuffer(draw_call_ui->command_buffer);
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
  void OnClick(float screen_x, float screen_y) {
    auto cell = _GetCellAtPoint(screen_x, screen_y);
    if (!cell->nodes.empty()) {
      (*(cell->nodes.begin()))->OnPressed();
    }
  }
  void OnHover(float screen_x, float screen_y) {
    auto cell = _GetCellAtPoint(screen_x, screen_y);
    if (!cell->nodes.empty()) {
      (*(cell->nodes.begin()))->OnHover();
    }
  }
  pair<unique_ptr<BatchDataUI>, MeshDataUI*> GetTextMesh(
    string& text,
    float screen_x,
    float screen_y,
    int font_size
  ) {
    // create UI mesh pointer
    unique_ptr<BatchDataUI> batch = font_manager->GetMeshTextUI(text, {255.0, 0.0, 0.0}, font_size);
    MeshDataUI* mesh_data = mesh_manager->CreateMeshDataFontUI(text, vec2(screen_x, screen_y));
    mesh_data->texture = batch->texture;

    return {std::move(batch), mesh_data};
  }
  void SubmitDraw(BatchDataUI* batch, MeshDataUI* mesh) {
      buffer->BufferMeshData(mesh, batch->vertices, batch->indices,
                               batch->colors, batch->uvs, {0, 0, 0},
                               mesh->texture);
      buffer->BufferTransform(mesh);
      buffer->AddCommand(mesh->command, draw_call_ui->command_buffer);
      drawn_ui_elements.push_back(mesh);
      draw_call_ui->command_count = 1;
  }
  UINode* AddInteractiveElement(
    MeshDataUI* mesh,
    //BatchDataUI* batch,
    int x_max, int x_min, int y_max, int y_min, 
    AnchorRelativeToNodePosition::AnchorRelativeToNodePosition anchor
  ) {
    // create UI node object.
    Point anchor_position = {mesh->transform.x, mesh->transform.y};
    UINodePosition node_position = {
      anchor_position,
      {anchor},
      x_max - x_min,
      y_max - y_min};
    ScreenCell* start_cell = _GetCellAtPoint(mesh->transform.x, mesh->transform.y);
    UINode* ui_node = new UINode(node_position, start_cell, 0);
    interactive_ui_elements.push_back(ui_node);
    // assign node to screen cells.
    _IterNodeCells(ui_node, _InsertNodeSetMaxOpenOrderCallback(ui_node));
    return ui_node;
  }
private:
  inline ScreenCell* _GetCellAtPoint(float screen_x, float screen_y) {
    if (!ControllerUtils::IsInScreenBorders(screen_x, screen_y)) {
      return nullptr;
    }
    return &all_cells[
      (int)(screen_x / cell_width) * total_cells_x +
        (int)(screen_y / cell_height)];
  }
  inline ScreenCell* _GetCellAtPointById(int screen_x, int screen_y) {
    return &all_cells[screen_x * total_cells_x + screen_y];
  }
  inline void _RemoveUINode(UINode* ui_node) {
    _IterNodeCells(ui_node, _RemoveNodeCallback(ui_node));
  }
  inline std::function<void(ScreenCell*)> _RemoveNodeCallback(UINode* ui_node) {
    return [ui_node](ScreenCell* cell) {
      cell->nodes.erase(ui_node);
    };
  }
  inline std::function<void(ScreenCell*)> _InsertNodeSetMaxOpenOrderCallback(UINode* ui_node) {
    return [ui_node](ScreenCell* cell) {
      // Must assign open_order before insert.
      ui_node->open_order = std::max(ui_node->open_order, (int)cell->nodes.size());
      cell->nodes.insert(ui_node);
    };
  }
  inline void _IterNodeCells(UINode* ui_node, std::function<void(ScreenCell*)> CellCallBack) {
    auto iter_x_max = std::min(ui_node->node_position.anchor_position.x + ui_node->node_position.width, GameSettings::SCR_WIDTH);
    auto iter_y_max = std::min(ui_node->node_position.anchor_position.y + ui_node->node_position.height, GameSettings::SCR_HEIGHT);
    for (int x = ui_node->start_cell->pos.x; x < iter_x_max; x+=cell_width) {
      for (int y = ui_node->start_cell->pos.y; y < iter_y_max; y+=cell_height) {
        CellCallBack(_GetCellAtPoint(x, y));
      }
    }
  }
  void _InitScreenLayout() {
    init_screen_width = GameSettings::SCR_WIDTH;
    init_screen_height = GameSettings::SCR_HEIGHT;
    total_cells_x = GameSettings::SCR_WIDTH/cell_width;
    total_cells_y = GameSettings::SCR_HEIGHT/cell_height;
    all_cells = new ScreenCell[total_cells_x * total_cells_y];
    for (int w = 0; w < total_cells_x; w++) {
      for (int h = 0; h < total_cells_y; h++)  {
        auto cell = _GetCellAtPointById(w, h);
        cell->pos.x = w * cell_width;
        cell->pos.y = h * cell_height;
        if (w != total_cells_x - 1) {
          cell->right = _GetCellAtPointById(w + 1, h);
        }
        if (h != total_cells_y - 1) {
          cell->top = _GetCellAtPointById(w, h + 1);
        }
        if (w != 0) {
          cell->left = _GetCellAtPointById(w - 1, h);
        }
        if (h != 0) {
          cell->bottom = _GetCellAtPointById(w, h - 1);
        }
      }
    }
  }
};
