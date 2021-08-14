#pragma once

#include "../sausage.h"
#include "Camera.h"
#include "../Settings.h"
#include "../buffer/BufferStorage.h"
#include "../OpenGLHelpers.h"
#include "../Shader.h"
#include "../Structures.h"
#include "Gui.h"
#include "../TestShapes.h"
#include "../utils/ThreadSafeQueue.h"

using namespace std;

class Renderer {
private:
	ThreadSafeQueue<pair<function<void()>, bool>> gl_commands;

	map<unsigned int, Shader*> shaders;
	map<unsigned int, vector<DrawCall*>> buffer_to_draw_call;
	set<pair<int, int>> buf_shad_ids;
public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_GLContext context;
	Renderer() {};
	~Renderer() {};
	void Render(Camera* camera);
	void RemoveBuffer(BufferStorage* buffer);
	void AddGlCommand(function<void()>& f, bool is_persistent);
	Shader* RegisterShader(const char* vs_name, const char* fs_name);
	bool AddDraw(DrawCall* draw);
	bool RemoveDraw(DrawCall* draw);
	void InitContext();
	void ClearContext();
private:
	void _ExecuteCommands();
};