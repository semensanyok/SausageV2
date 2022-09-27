#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "BufferStorage.h"
#include "OpenGLHelpers.h"
#include "Shader.h"
#include "Structures.h"
#include "ImguiGui.h"
#include "TestShapes.h"
#include "ThreadSafeQueue.h"
#include "BufferConsumer.h"
#include "RendererContextManager.h"
#include "DrawCallManager.h"

using namespace std;

class Renderer : public SausageSystem {
private:
	RendererContextManager* context_manager;
    BufferStorage* buffer;
	ThreadSafeQueue<pair<function<void()>, bool>> gl_commands;

  map<DrawOrder::DrawOrder, vector<DrawCall*>> draw_calls;
public:
	SDL_Renderer* renderer;
	Renderer(
		RendererContextManager* context_manager,
        BufferStorage* buffer
  ) : context_manager{ context_manager }, buffer{ buffer }{};
	~Renderer() {};
	void Render(Camera* camera);
	void AddGlCommand(function<void()>& f, bool is_persistent);
	bool AddDraw(DrawCall* draw, DrawOrder::DrawOrder draw_order);
	bool RemoveDraw(DrawCall* draw, DrawOrder::DrawOrder draw_order);
private:
	void _ExecuteCommands();
};
