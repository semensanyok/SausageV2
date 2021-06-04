#pragma once

#include "sausage.h"
#include "Camera.h"
#include "Settings.h"
#include "BufferStorage.h"
#include "OpenGLHelpers.h"
#include "Shader.h"
#include "Structures.h"
#include "Gui.h"

class Renderer {
public:
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_GLContext context;

	map<unsigned int, Shader*> shaders;
	map<unsigned int, vector<BufferStorage*>> shader_render_queue;

	Renderer() {};
	~Renderer() {};

	void Render(Camera* camera)
	{
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		RenderGui(window, camera);

		for (auto draw : shader_render_queue) {
			if (!draw.second.empty()) {
				glUseProgram(draw.first);
			}
			for (BufferStorage* buffer : draw.second) {
				if (buffer->active_commands_to_render > 0) {
					buffer->BarrierIfChange();
					buffer->BindMeshVAOandBuffers();
					glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, nullptr, buffer->active_commands_to_render, 0);
					//glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, buffer->active_commands.data(), buffer->active_commands_to_render, 0);
					buffer->fence_sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
					CheckGLError();
				}
			}
		}
		SDL_GL_SwapWindow(window);
	}

	Shader* RegisterShader(const char* vs_name, const char* fs_name) {
		Shader* shader = new Shader(GetShaderPath(vs_name), GetShaderPath(fs_name));
		shaders[shader->id] = shader;
		shader_render_queue[shader->id] = vector<BufferStorage*>();
		return shader;
	}

	bool AddDraw(Shader* shader, BufferStorage* buffers) {
		if (!shader_render_queue.contains(shader->id)) {
			LOG((ostringstream() << "Unable to add draw for unregistered shader: " << shader).str());
			return false;
		}
		shader_render_queue[shader->id].push_back(buffers);
		return true;
	}

	bool RemoveDraw(BufferStorage* buffers_to_remove, Shader* shader = nullptr) {
		if (shader != nullptr && shader_render_queue.contains(shader->id)) {
			auto buffers = shader_render_queue[shader->id];
			RemoveDrawFromShader(buffers, buffers_to_remove->id);
		}
		else {
			for (auto buffers : shader_render_queue) {
				RemoveDrawFromShader(buffers.second, buffers_to_remove->id);
			}
		}
	}

	void RemoveDrawFromShader(vector<BufferStorage*>& buffers, unsigned int buffer_id) {
		auto iter = buffers.begin();
		while (iter != buffers.end()) {
			if ((*iter)->id == buffer_id) {
				iter = buffers.erase(iter);
			}
			else {
				iter++;
			}
		}
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

		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glEnable(GL_DEPTH_TEST);
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
};