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

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

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
	void Draw();
};

void InitContext() {
	SDL_Init(SDL_INIT_VIDEO);

	//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

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

void CleanupGui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}

void ClearContext() {
	WriteShaderMsgsToLogFile();
	CleanupGui();
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
	camera = new Camera(80.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0.0f, 0.0f, -10.0f));
	samplers = InitSamplers();
	textures = _LoadTextures();
	//_DebugDraw();
	_ModelDraw();
}

void InitGui() {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::StyleColorsDark();
	ImGui_ImplSDL2_InitForOpenGL(window, context);
	const char* glsl_version = "#version 460";
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void RenderGui() {
	static bool show_demo_window = true;
	static bool show_another_window = true;
	
	
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame(window);
	ImGui::NewFrame();
	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	{
		static float f = 0.0f;
		static int counter = 0;
		static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &show_another_window);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (show_another_window)
	{
		ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			show_another_window = false;
		ImGui::End();
	}
	ImGui::Render();
	//glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
	//glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
	//glClear(GL_COLOR_BUFFER_BIT);
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
			ImGui_ImplSDL2_ProcessEvent(&e);
			ProcessEvent(&e);
		}
		Render();
		RenderGui();
	}
	ClearContext();
	log_thread.join();
	return 0;
}
