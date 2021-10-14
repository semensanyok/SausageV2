#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "BufferStorage.h"
#include "OpenGLHelpers.h"
#include "Shader.h"
#include "Structures.h"
#include "Gui.h"
#include "TestShapes.h"
#include "ThreadSafeQueue.h"
#include "BufferConsumer.h"
#include "RendererContextManager.h"

using namespace std;

class Renderer {
private:
	RendererContextManager* context_manager;
	ThreadSafeQueue<pair<function<void()>, bool>> gl_commands;

	map<unsigned int, Shader*> shaders;
	map<unsigned int, map<DrawOrder, vector<DrawCall*>>> buffer_to_draw_call;
	set<pair<int, int>> buf_shad_ids;
public:
	SDL_Renderer* renderer;
	Renderer(
		RendererContextManager* context_manager
	) : context_manager{ context_manager } {};
	~Renderer() {};
	void Render(Camera* camera);
	void RemoveBuffer(BufferStorage* buffer);
	void AddGlCommand(function<void()>& f, bool is_persistent);
	Shader* RegisterShader(const char* vs_name, const char* fs_name);
	bool AddDraw(DrawCall* draw, DrawOrder draw_order);
	bool RemoveDraw(DrawCall* draw, DrawOrder draw_order);
private:
	void _ExecuteCommands();
};
