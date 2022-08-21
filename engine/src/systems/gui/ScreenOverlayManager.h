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

class PauseMenuSettings {
public:
  int button_font_size;
  int back_indent;
  int button_width;
  int button_height;
  vec3 text_color;
  vec3 back_color;
  PauseMenuSettings(
    int button_font_size,
    int back_indent,
    vec3 text_color,
    vec3 back_color) :
    button_font_size{button_font_size},
    back_indent{back_indent},
    text_color{text_color},
    back_color{back_color} {
    button_width = button_font_size * 4 + 2 * back_indent;
    button_height = button_font_size + 2 * back_indent;
  }
};
  

class UINode
{
  friend class ScreenOverlayManager;
  MeshDataUI* text;
  MeshDataUI* background;
  UINodePosition node_position;
  ScreenCell* start_cell;
  //vector<function<void()>> on_destroys;
public:
  // suborder. expected to be max for text and min for background, to draw text above background.
  int open_order;
  void Call() { cout << "Call" << endl; };
  void OnHover() { cout << "OnHover" << endl; };
  void OnPressed() { cout << "OnPressed" << endl; };
  void OnReleased() { cout << "OnReleased" << endl; };
  void OnDestroy() {};
private:
  UINode(
    UINodePosition node_position,
    ScreenCell* start_cell,
    int open_order,
    MeshDataUI* text,
    MeshDataUI* background) :
    node_position{node_position},
    start_cell{start_cell},
    open_order {open_order},
    text {text},
    background {background} {}
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

class ScreenOverlayManager : public SausageSystem
{
  friend class SausageTestBase;
  friend class ScreenOverlayManagerTest;
private:
  int init_screen_width, init_screen_height;
  int cell_width = 10, cell_height = 10;
  int total_cells_x, total_cells_y;
  ScreenCell* all_cells;

  map<int, set<UINode*, decltype([](UINode* lhs, UINode* rhs) {
        return lhs->open_order > rhs->open_order;
    })>> open_order_to_nodes;

  vector<MeshDataUI*> drawn_ui_elements;
  vector<UINode*> active_ui_elements;

  UIBufferConsumer* buffer;
  MeshManager* mesh_manager;
  FontManager* font_manager;
  DrawCall* draw_call_text;
  DrawCall* draw_call_back;
  CommandBuffer* command_buffer_font;
  CommandBuffer* command_buffer_back;
  Renderer* renderer;

  const unsigned int COMMAND_BUFFER_SIZE = 100;
  int total_draw_commands_text = 0;
  int total_draw_commands_back = 0;

  bool is_pause_menu_active = false;
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
    command_buffer_font = buffer->CreateCommandBuffer(COMMAND_BUFFER_SIZE);
    command_buffer_back = buffer->CreateCommandBuffer(COMMAND_BUFFER_SIZE);

    draw_call_text = new DrawCall();
    draw_call_text->shader = shaders->font_ui;
    draw_call_text->mode = GL_TRIANGLES;
    draw_call_text->buffer = (BufferConsumer*)buffer;
    draw_call_text->command_buffer = command_buffer_font;

    draw_call_back = new DrawCall();
    draw_call_back->shader = shaders->back_ui;
    draw_call_back->mode = GL_TRIANGLES;
    draw_call_back->buffer = (BufferConsumer*)buffer;
    draw_call_back->command_buffer = command_buffer_back;
  };
  ~ScreenOverlayManager() {
    Deactivate();
  }
  void Init() {
    _InitScreenLayout();
    renderer->AddDraw(draw_call_text, DrawOrder::UI_TEXT);
    draw_call_text->buffer->ActivateCommandBuffer(draw_call_text->command_buffer);
    renderer->AddDraw(draw_call_back, DrawOrder::UI_BACK);
    draw_call_back->buffer->ActivateCommandBuffer(draw_call_back->command_buffer);
  }
  void Deactivate() {
    renderer->RemoveDraw(draw_call_text, DrawOrder::UI_TEXT);
    draw_call_text->buffer->RemoveCommandBuffer(draw_call_text->command_buffer);
    renderer->RemoveDraw(draw_call_back, DrawOrder::UI_BACK);
    draw_call_back->buffer->RemoveCommandBuffer(draw_call_back->command_buffer);
  }
  void KeyCallback(int scan_code) {
    if (scan_code == KeyboardLayout::PauseMenu) {
        if (is_pause_menu_active) {
          DectivatePauseMenu();
        } else {
          ActivatePauseMenu();
        }
    }
  }
  void OnResize() { // NOT TESTED
    float height_delta = (float)GameSettings::SCR_HEIGHT / init_screen_height;
    float width_delta = (float)GameSettings::SCR_WIDTH / init_screen_width;
    for (auto ui_node : active_ui_elements) {
      _IterNodeCells(ui_node, _RemoveNodeCallback(ui_node));
      ui_node->node_position.anchor_position.x *= width_delta;
      ui_node->node_position.anchor_position.y *= height_delta;
      ui_node->node_position.height *= height_delta;
      ui_node->node_position.width *= width_delta;
    }
    init_screen_width = GameSettings::SCR_WIDTH;
    init_screen_height = GameSettings::SCR_HEIGHT;
    cell_width = cell_width * width_delta;
    cell_height = cell_height * height_delta;
    for (int w = 0; w < total_cells_x; w++) {
      for (int h = 0; h < total_cells_y; h++)  {
        auto cell = _GetCellAtPointById(w, h);
        cell->pos.x *= width_delta;
        cell->pos.y *= height_delta;
      }
    }
    for (auto ui_node : active_ui_elements) {
      _IterNodeCells(ui_node, _InsertNodeCallback(ui_node));
    }
    // TODO: pass scale to shader
  }
  void OnClick(float screen_x, float screen_y) {
    if (!is_pause_menu_active) {
      return;
    }
    auto cell = _GetCellAtPoint(screen_x, screen_y);
    if (cell != nullptr && !cell->nodes.empty()) {
      (*(cell->nodes.begin()))->OnPressed();
    }
  }
  void OnHover(float screen_x, float screen_y) {
    if (!is_pause_menu_active) {
      return;
    }
    auto cell = _GetCellAtPoint(screen_x, screen_y);
    if (cell != nullptr && !cell->nodes.empty()) {
      (*(cell->nodes.begin()))->OnHover();
    }
  }
  void InitPauseMenu(PauseMenuSettings pause_menu_settings = {FontSizes::STANDART, FontSizes::STANDART / 2, {255,0,0},{0,0,0}}) {
    const int button_font_size = pause_menu_settings.button_font_size;
    const int back_indent = pause_menu_settings.back_indent;
    const int button_width = pause_menu_settings.button_width;
    const int button_height = pause_menu_settings.button_height;
    const vec3 text_color = pause_menu_settings.text_color;
    const vec3 back_color = pause_menu_settings.back_color;
    int init_open_order = 0;

    const vector<string> buttons = {
      "quit",
      "settings",
      "load",
      "save",
      "resume",
    };
    int menu_start_x = GameSettings::SCR_WIDTH / 2 - button_width / 2;
    int menu_start_y = GameSettings::SCR_HEIGHT / 2 - (button_height * buttons.size()) / 2;
    for (int i = 0; i < buttons.size(); i++) {
      auto b_start_x = menu_start_x;
      auto b_start_y = menu_start_y + i * button_height;
      auto t_start_x = b_start_x + back_indent;
      auto t_start_y = b_start_y + back_indent;

      auto back = _GetBackgroundMesh(back_color, b_start_x, b_start_y, button_width, button_height);
      auto text = _GetTextMesh(buttons[i], text_color, t_start_x, t_start_y, button_font_size);
      auto back_mesh = back.second;
      auto text_mesh = text.second;

      _SubmitDrawText(text.first.get(), text_mesh);
      _SubmitDrawBack(back.first.get(),back_mesh);

      _AddUINode(text_mesh, back_mesh,
                 back.first->x_max,
                 back.first->x_min,
                 back.first->y_max,
                 back.first->y_min,
                 AnchorRelativeToNodePosition::LeftBottom,
                 init_open_order);
    }
  }
  void ActivatePauseMenu() {
    draw_call_text->command_count = total_draw_commands_text;
    draw_call_back->command_count = total_draw_commands_back;
    is_pause_menu_active = true;
  }
  void DectivatePauseMenu() {
    draw_call_text->command_count = 0;
    draw_call_back->command_count = 0;
    is_pause_menu_active = false;
  }
private:
  void _SubmitDrawText(
    BatchDataUI* batch,
    MeshDataUI* mesh
  ) {
      buffer->BufferMeshData(mesh, batch->vertices, batch->indices, batch->colors, batch->uvs);
      buffer->BufferTransform(mesh);
      buffer->BufferSize(mesh, batch->x_min, batch->x_max, batch->y_min, batch->y_max);
      buffer->AddCommand(mesh->command, draw_call_text->command_buffer,total_draw_commands_text++);
      drawn_ui_elements.push_back(mesh);
  }
  void _SubmitDrawBack(
    BatchDataUI* batch,
    MeshDataUI* mesh
  ) {
      buffer->BufferMeshData(mesh, batch->vertices, batch->indices, batch->colors, batch->uvs);
      buffer->BufferTransform(mesh);
      auto min = mesh->transform;
      min.x += batch->x_min;
      min.y += batch->y_min;
      auto max = mesh->transform;
      max.x += batch->x_max;
      max.y += batch->y_max;

      buffer->BufferSize(mesh, min.x, max.x, min.y, max.y);
      buffer->AddCommand(mesh->command, draw_call_back->command_buffer,total_draw_commands_back++);
      drawn_ui_elements.push_back(mesh);
  }
  pair<unique_ptr<BatchDataUI>, MeshDataUI*> _GetTextMesh(
    string text,
    vec3 color,
    float screen_x, float screen_y,
    int font_size
  ) {
    // create UI mesh pointer
    unique_ptr<BatchDataUI> batch = font_manager->GetMeshTextUI(text, color, font_size);
    MeshDataUI* mesh_data = mesh_manager->CreateMeshDataFontUI(vec2(screen_x, screen_y),batch->texture);
    mesh_data->texture = batch->texture;

    return {std::move(batch), mesh_data};
  }
  // do enhancements in the shader. here just set base color.
  pair<unique_ptr<BatchDataUI>, MeshDataUI*> _GetBackgroundMesh(
    vec3 color,
    float screen_x, float screen_y,
    int size_x, int size_y) {
    auto batch = make_unique<BatchDataUI>();
    batch->vertices = {{0,0,-1},{0,size_y,-1},{size_x,0,-1},{size_x,size_y,-1}};
    batch->indices = {1,0,2,1,2,3};
    batch->colors = {color,color,color,color};

    batch->uvs.push_back({0, size_y});
    batch->uvs.push_back({0, 0});
    batch->uvs.push_back({size_x, size_y});
    batch->uvs.push_back({size_x, 0});

    batch->x_min = 0;
    batch->x_max = size_x;
    batch->y_min = 0;
    batch->y_max = size_y;

    MeshDataUI* mesh_data = mesh_manager->CreateMeshDataFontUI(vec2(screen_x, screen_y));
    return {std::move(batch), mesh_data};
  }
  UINode* _AddUINode(
    MeshDataUI* text,
    MeshDataUI* background,
    //BatchDataUI* batch,
    int x_max, int x_min, int y_max, int y_min, 
    AnchorRelativeToNodePosition::AnchorRelativeToNodePosition anchor,
    int open_order
  ) {
    auto biggest_mesh = background == nullptr ? text : background;
    Point anchor_position = {biggest_mesh->transform.x, biggest_mesh->transform.y};
    UINodePosition node_position = {
      anchor_position,
      {anchor},
      x_max - x_min,
      y_max - y_min};
    ScreenCell* start_cell = _GetCellAtPoint(biggest_mesh->transform.x, biggest_mesh->transform.y);
    UINode* ui_node = new UINode(node_position, start_cell, open_order, text, background);
    active_ui_elements.push_back(ui_node);
    // assign node to screen cells.
    _IterNodeCells(ui_node, _InsertNodeCallback(ui_node));
    return ui_node;
  }
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
  inline std::function<void(ScreenCell*)> _RemoveNodeCallback(UINode* ui_node) {
    return [ui_node](ScreenCell* cell) {
      cell->nodes.erase(ui_node);
    };
  }
  inline std::function<void(ScreenCell*)> _InsertNodeCallback(UINode* ui_node) {
    return [ui_node](ScreenCell* cell) {
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
