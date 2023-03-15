#pragma once

#include "sausage.h"
#include "Settings.h"
#include "OpenGLHelpers.h"
#include "ThreadSafeQueue.h"
#include "Macros.h"
#include "Structures.h"
#include "DrawCallStruct.h"
#include "Logging.h"
#include "Camera.h"
#include "BufferStorage.h"
#include "Shader.h"
#include "ImguiGui.h"
#include "BufferConsumer.h"
#include "RendererContextManager.h"
#include "GLCommandBuffers.h"
#include "GLVertexAttributes.h"
#include "BufferManager.h"

using namespace std;
/**
 * Responsible for issuing all GL commands
 * must be called from main thread (gl context thread)
*/
class Renderer : public SausageSystem {
private:
  RendererContextManager* context_manager;
  BufferManager* buffer_manager;
  GLVertexAttributes* vertex_attributes;

  ThreadSafeQueue<pair<function<void()>, bool>> gl_commands;

  map<DrawOrder, unordered_set<DrawCall*>> draw_calls;
public:
  Renderer(
    RendererContextManager* context_manager,
    BufferManager* buffer_manager
  ) :
    context_manager{ context_manager },
    buffer_manager{ buffer_manager },
    vertex_attributes{ buffer_manager->vertex_attributes } {};
  ~Renderer() {};
  void Render(Camera* camera);
  void AddGlCommand(function<void()>& f, bool is_persistent);
  bool AddDraw(DrawCall* draw, DrawOrder draw_order);
  bool RemoveDraw(DrawCall* draw, DrawOrder draw_order);
private:
  void _ExecuteCommands();
};
