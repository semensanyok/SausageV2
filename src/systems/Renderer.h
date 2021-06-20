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
#include "utils/ThreadSafeQueue.h"

class Renderer {
private:
	ThreadSafeQueue<function<void()>> gl_commands;

	map<unsigned int, Shader*> shaders;
	map<unsigned int, vector<DrawCall*>> buffer_shaders;
	set<pair<int, int>> buf_shad_ids;
public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_GLContext context;

	Renderer() {};
	~Renderer() {};

	void Render(Camera* camera)
	{
		_ExecuteCommands();
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		for (auto buffer_shader : buffer_shaders) {
			if (buffer_shader.second.empty()) {
				continue;
			}
			auto buffer = (*buffer_shader.second.begin())->buffer;
			buffer->BarrierIfChangeAndUnmap();
			buffer->BindMeshVAOandBuffers(); // TODO: one buffer, no rebind
			for (auto draw : buffer_shader.second) {
				if (draw->buffer->active_commands_to_render > 0) {
					glUseProgram(draw->shader->id);
					draw->shader->setMat4("projection_view", camera->projection_view);
					draw->shader->setVec3("view_pos", camera->pos);

					glMultiDrawElementsIndirect(draw->mode, GL_UNSIGNED_INT, nullptr, draw->command_count, draw->command_offset);
					//glMultiDrawArraysIndirect(draw->mode, nullptr, draw->command_count, draw->command_offset);
				}
			}
			buffer->fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
			CheckGLError();
		}
		RenderGui(window, camera);
		SDL_GL_SwapWindow(window);
	}

	void RemoveBuffer(BufferStorage* buffer) {
		auto buf = buffer_shaders.find(buffer->id);
		if (buf != buffer_shaders.end()) {
			for (auto draw : (*buf).second) {
				buf_shad_ids.erase(pair(draw->buffer->id, draw->shader->id));
				delete draw;
			}
			buffer_shaders.erase(buf);
		}
	}

	void AddGlCommand(function<void()>& f)
	{
		gl_commands.Push(f);
	}

	Shader* RegisterShader(const char* vs_name, const char* fs_name) {
		for (auto shader : shaders) {
			if (shader.second->vertex_path.ends_with(vs_name) || shader.second->fragment_path.ends_with(fs_name)) {
				LOG((ostringstream() << "Shader with vs_name=" << string(vs_name) << " fs_name=" << string(fs_name) << " already registered").str());
			}
			return shader.second;
		}
		Shader* shader = new Shader(GetShaderPath(vs_name), GetShaderPath(fs_name));
		shader->InitOrReload();
		shaders[shader->id] = shader;
		return shader;
	}

	bool AddDraw(DrawCall* draw) {
		if (!shaders.contains(draw->shader->id)) {
			LOG((ostringstream() << "Unable to add draw for unregistered shader: " << draw->shader->id << " vs:" << draw->shader->vertex_path << " fs:" << draw->shader->fragment_path).str());
			return false;
		}
		pair<int, int> buf_shad_id{ draw->buffer->id, draw->shader->id };
		if (buf_shad_ids.contains(buf_shad_id)) {
			LOG((ostringstream() << "Draw for shader: " << draw->shader->id << " buffer:" << draw->buffer->id << "already exists").str());
			return false;
		}
		buffer_shaders[draw->buffer->id].push_back(draw);
		buf_shad_ids.insert(buf_shad_id);
		return true;
	}

	void InitContext() {
		SDL_Init(SDL_INIT_VIDEO);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

		//SDL_ShowCursor(SDL_ENABLE);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
		SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef GL_DEBUG
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

		SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

		window = SDL_CreateWindow("caveview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			SCR_WIDTH, SCR_HEIGHT,
			SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI // SDL_WINDOW_VULKAN
		);
		if (!window) LOG("Couldn't create window");

		context = SDL_GL_CreateContext(window);
		if (!context) LOG("Couldn't create context");

		SDL_GL_MakeCurrent(window, context); // is this true by default?

		// hide cursor, report only mouse motion events
		// SDL_SetRelativeMouseMode(SDL_TRUE);

		// enable VSync
		SDL_GL_SetSwapInterval(1);

		if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
		{
			LOG("[ERROR] Couldn't initialize glad");
		}
		else
		{
			LOG("[INFO] glad initialized\n");
		}

		glEnable(GL_DEPTH_TEST);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		// debug
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
		
		InitGuiContext(window, context);
	}

	void ClearContext() {
		CleanupGui();

		WriteShaderMsgsToLogFile();
		SDL_GL_DeleteContext(context);
		SDL_DestroyWindow(window);
		SDL_Quit();
	}
private:
	void _ExecuteCommands()
	{
		function<void()> f;
		auto commands = gl_commands.PopAll();
		while (!commands.empty()) {
			auto& command = commands.front();
			command();
			commands.pop();
		}
	}
};