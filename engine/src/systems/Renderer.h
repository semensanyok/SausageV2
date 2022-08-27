#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "BufferStorage.h"
#include "OpenGLHelpers.h"
#include "Shader.h"
#include "structures/Structures.h"
#include "ImguiGui.h"
#include "TestShapes.h"
#include "ThreadSafeQueue.h"
#include "BufferConsumer.h"
#include "RendererContextManager.h"

using namespace std;

class Renderer : public SausageSystem {
private:
	RendererContextManager* context_manager;
	ThreadSafeQueue<pair<function<void()>, bool>> gl_commands;

	map<unsigned int, map<DrawOrder::DrawOrder, vector<DrawCall*>>> buffer_to_draw_call;
public:
	SDL_Renderer* renderer;
	Renderer(
		RendererContextManager* context_manager
	) : context_manager{ context_manager } {};
	~Renderer() {};
	void Render(Camera* camera);
	void RemoveBuffer(BufferStorage* buffer);
	void AddGlCommand(function<void()>& f, bool is_persistent);
	bool AddDraw(DrawCall* draw, DrawOrder::DrawOrder draw_order);
	bool RemoveDraw(DrawCall* draw, DrawOrder::DrawOrder draw_order);
private:
	void _ExecuteCommands();
};
