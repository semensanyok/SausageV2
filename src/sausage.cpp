// sausage.cpp : Defines the entry point for the application.
//

#include "sausage.h"
#include "Settings.h"
#include "Mesh.h"
#include "Camera.h"
#include "Settings.h"
#include "Shader.h"
#include "AssetUtils.h"
#include "Logging.h"

using namespace std;
using namespace glm;

// SDL state
SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext context;

// Game state
Camera* camera;
vector<Mesh*> meshes;
Samplers* samplers;
float delta_time = 0;
float last_ticks = 0;
bool quit = false;

vector<Texture*> textures;

//// GUI state

struct Draw {
	// 1) location or vector of locs
	// 2) 1 shader many meshes
	// 3) instanced draw - same mesh same shader multiple draw different pos
	Mesh* mesh;
	Shader* shader;
	vector<Texture*> textures;
};

vector<Draw> draws;

class SceneNode {
	vec3 position;
	vector<SceneNode> children;
	virtual void Draw() = 0;
};

void InitContext() {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);

#ifdef GL_DEBUG
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	
	window = SDL_CreateWindow("caveview", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		SCR_WIDTH, SCR_HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE // SDL_WINDOW_VULKAN
	);
	if (!window) LOG("Couldn't create window");

	context = SDL_GL_CreateContext(window);
	if (!context) LOG("Couldn't create context");

	SDL_GL_MakeCurrent(window, context); // is this true by default?

	SDL_SetRelativeMouseMode(SDL_TRUE);
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
}

void ClearContext() {
	WriteShaderMsgsToLogFile();
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void ProcessEvent(SDL_Event* e)
{
	switch (e->type) {
	case SDL_MOUSEMOTION:
		camera->MouseCallback(e);
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
		camera->KeyCallback(s, delta_time);
		break;
	}
	case SDL_KEYUP: {
		break;
	}
	}
}

Mesh* GetPlane() {
	static  std::vector<float> vertices = {
		0.5f,  0.5f, 0.0f,  // top right
		0.5f, -0.5f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  // bottom left
		-0.5f,  0.5f, 0.0f   // top left 
	};
	static  std::vector<unsigned int> indices = {
		0, 1, 3,  // first Triangle
		1, 2, 3   // second Triangle
	};
	Mesh* mesh = CreateMesh(vertices, indices);
	return mesh;
}

vector<Texture*> _LoadTextures() {
	vector<Texture*> textures;
	Texture* texture_diffuse = LoadTexture(GetTexturePath<const char*>("Image0000.png").c_str(), "texture_diffuse", TextureType::Diffuse);
	Texture* texture_normal = LoadTexture(GetTexturePath<const char*>("Image0000_normal.png").c_str(), "texture_normal", TextureType::Normal);
	Texture* texture_specular = LoadTexture(GetTexturePath<const char*>("Image0000_specular.png").c_str(), "texture_specular", TextureType::Specular);
	Texture* texture_height = LoadTexture(GetTexturePath<const char*>("Image0000_height.png").c_str(), "texture_height", TextureType::Height);

	if (texture_diffuse != nullptr) {
		textures.push_back(texture_diffuse);
	}
	if (texture_normal != nullptr) {
		textures.push_back(texture_normal);
	}
	if (texture_specular != nullptr) {
		textures.push_back(texture_specular);
	}
	if (texture_height != nullptr) {
		textures.push_back(texture_height);
	}
	return textures;
}

void _ModelDraw() {
	Shader* shader = new Shader{ GetShaderPath("model_vs.glsl"), GetShaderPath("model_fs.glsl") };

	LoadMeshes(meshes, GetModelPath("cube.fbx"));
	for_each(meshes.begin(), meshes.end(), [&](Mesh* mesh) {
		InitBuffers(mesh);
		draws.push_back(Draw{ mesh, shader, textures });
		});
}

void _DebugDraw() {
	Mesh* mesh = GetPlane();
	InitBuffers(mesh);
	draws.push_back(Draw{ mesh, new Shader{ GetShaderPath("debug_vs.glsl"), GetShaderPath("debug_fs.glsl") } });
}

void InitGame() {
	camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 0.0f, 1.0f));
	samplers = InitSamplers();
	textures = _LoadTextures();
	//_DebugDraw();
	_ModelDraw();
}

void Render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (Draw draw : draws) {
		draw.shader->use();
		draw.shader->setMat4("mvp",
			camera->projection_matrix * 
			camera->view_matrix * 
			mat4(1));
		// shader->setMat4("model", draw->node.model_transform);
		for (Texture* texture : draw.textures) {
			BindTexture(texture, draw.shader->id);
			glBindSampler((int)texture->type, samplers->basic_repeat);
			auto asd1 = mat4(1);
			// LogShaderFull(draw.shader->id);
		}
		glDrawElements(GL_TRIANGLES, draw.mesh->indices.size(), GL_UNSIGNED_INT, NULL);
	}
	// Draw
	SDL_GL_SwapWindow(window);
}

void _UpdateDeltaTime() {
	float this_ticks = SDL_GetTicks();
	delta_time = this_ticks - last_ticks;
	last_ticks = this_ticks;
}


int SDL_main(int argc, char** argv)
{
	auto log_thread = LogIO(quit);
	InitContext();
	InitGame();
	while (!quit) {
		_UpdateDeltaTime();
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ProcessEvent(&e);
		}
		Render();
	}
	ClearContext();
	log_thread.join();
	return 0;
}
