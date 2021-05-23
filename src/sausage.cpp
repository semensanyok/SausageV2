// sausage.cpp : Defines the entry point for the application.
//

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include "sausage.h"
#include "systems/Renderer.h"
#include "systems/Gui.h"
#include "Settings.h"
#include "Mesh.h"
#include "Settings.h"
#include "Shader.h"
#include "AssetUtils.h"
#include "Logging.h"
#include "systems/Camera.h"
#include "systems/Gui.h"
#include "systems/BufferStorage.h"

using namespace std;
using namespace glm;

// Systems
Camera camera;
BufferStorage buffer_m;
Renderer renderer;

// Game state
Samplers* samplers;
float delta_time = 0;
float last_ticks = 0;
bool quit = false;

void ProcessEvent(SDL_Event* e)
{
	switch (e->type) {
	case SDL_MOUSEMOTION:
		// camera->MouseMotionCallback(e);
		break;
	case SDL_MOUSEBUTTONDOWN:
		break;
	case SDL_MOUSEBUTTONUP:
		break;
	case SDL_MOUSEWHEEL:
		break;
	case SDL_QUIT:
		quit = 1;
		break;

	case SDL_WINDOWEVENT:
		switch (e->window.event) {
		case SDL_WINDOWEVENT_SIZE_CHANGED:
			SCR_HEIGHT = e->window.data1;
			SCR_WIDTH = e->window.data2;
			break;
		}
		break;

	case SDL_KEYDOWN: {
		int k = e->key.keysym.sym;
		int s = e->key.keysym.scancode;

		// Intercept SHIFT + ~ key stroke to toggle libRocket's 
					// visual debugger tool
		if (e->key.keysym.sym == SDLK_BACKQUOTE &&
			e->key.keysym.mod == KMOD_LSHIFT)
		{
			break;
		}

		SDL_Keymod mod;
		mod = SDL_GetModState();
		if (k == SDLK_ESCAPE)
			quit = 1;
		camera.KeyCallbackRTS(s, delta_time);
		break;
	}
	case SDL_KEYUP: {
		break;
	}
	}
}

vector<Texture*> _LoadTextures() {
	MeshData(LoadTextureArray("Image0001.png", "Image0000_normal.png", "Image0000_specular.png", "Image0000_height.png"));
	return textures;
}																			

void InitGame() {
	camera = new Camera(80.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 0.0f, 3.0f), 0.0f, -45.0f);
	samplers = InitSamplers();
}

void _UpdateDeltaTime() {
	float this_ticks = SDL_GetTicks();
	delta_time = this_ticks - last_ticks;
	last_ticks = this_ticks;
}

int SDL_main(int argc, char** argv)
{
	//auto log_thread = LogIO(quit);
	//InitContext();
	//InitGuiContext();

	//InitGame();

	//if (SDL_GL_ExtensionSupported("GL_ARB_bindless_texture")) {
	//	cout << "GL_ARB_bindless_texture" << endl;
	//}
	//else {
	//	cout << "you suck" << endl;
	//}

	//while (!quit) {
	//	_UpdateDeltaTime();
	//	SDL_Event e;
	//	while (SDL_PollEvent(&e)) {
	//		ImGui_ImplSDL2_ProcessEvent(&e);
	//		ProcessEvent(&e);
	//	}
	//	Render();
	//	RenderGui();
	//	// Draw
	//	SDL_GL_SwapWindow(window);
	//}
	//ClearContext();
	//log_thread.join();
	return 0;
}
