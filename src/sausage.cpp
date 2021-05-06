// sausage.cpp : Defines the entry point for the application.
//

#include "sausage.h"
#include "Settings.h"
#include "Mesh.h"
#include "Camera.h"
#include "Settings.h"
#include "Shader.h"
#include "AssetUtils.h"

#define LOG(s) cout<<s<<endl;

using namespace std;
using namespace glm;

// SDL state
SDL_Window* window;
SDL_Renderer* renderer;
SDL_GLContext context;

// Game state
Camera* camera;
vector<Mesh*> meshes;

struct Draw {
	// 1) location or vector of locs
	// 2) 1 shader many meshes
	// 3) instanced draw - same mesh same shader multiple draw different pos
	Mesh* mesh;
	Shader* shader;
};

vector<Draw> draws;

int quit = 0;

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
}

void ClearContext() {
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void LogShaderMessage(unsigned int id) {
	int max_length = 2048;
	int actual_length = 0;
	char program_log[2048];
	glGetProgramInfoLog(id, max_length, &actual_length, program_log);
	printf("program info log for GL index %u:\n%s", id, program_log);
}

const char* GL_type_to_string(GLenum type) {
	switch (type) {
	case GL_BOOL: return "bool";
	case GL_INT: return "int";
	case GL_FLOAT: return "float";
	case GL_FLOAT_VEC2: return "vec2";
	case GL_FLOAT_VEC3: return "vec3";
	case GL_FLOAT_VEC4: return "vec4";
	case GL_FLOAT_MAT2: return "mat2";
	case GL_FLOAT_MAT3: return "mat3";
	case GL_FLOAT_MAT4: return "mat4";
	case GL_SAMPLER_2D: return "sampler2D";
	case GL_SAMPLER_3D: return "sampler3D";
	case GL_SAMPLER_CUBE: return "samplerCube";
	case GL_SAMPLER_2D_SHADOW: return "sampler2DShadow";
	default: break;
	}
	return "other";
}
void print_all(GLuint programme) {
	printf("--------------------\nshader programme %i info:\n", programme);
	int params = -1;
	glGetProgramiv(programme, GL_LINK_STATUS, &params);
	printf("GL_LINK_STATUS = %i\n", params);

	glGetProgramiv(programme, GL_ATTACHED_SHADERS, &params);
	printf("GL_ATTACHED_SHADERS = %i\n", params);

	glGetProgramiv(programme, GL_ACTIVE_ATTRIBUTES, &params);
	printf("GL_ACTIVE_ATTRIBUTES = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveAttrib(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetAttribLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetAttribLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	glGetProgramiv(programme, GL_ACTIVE_UNIFORMS, &params);
	printf("GL_ACTIVE_UNIFORMS = %i\n", params);
	for (int i = 0; i < params; i++) {
		char name[64];
		int max_length = 64;
		int actual_length = 0;
		int size = 0;
		GLenum type;
		glGetActiveUniform(
			programme,
			i,
			max_length,
			&actual_length,
			&size,
			&type,
			name
		);
		if (size > 1) {
			for (int j = 0; j < size; j++) {
				char long_name[64];
				sprintf(long_name, "%s[%i]", name, j);
				int location = glGetUniformLocation(programme, long_name);
				printf("  %i) type:%s name:%s location:%i\n",
					i, GL_type_to_string(type), long_name, location);
			}
		}
		else {
			int location = glGetUniformLocation(programme, name);
			printf("  %i) type:%s name:%s location:%i\n",
				i, GL_type_to_string(type), name, location);
		}
	}

	LogShaderMessage(programme);
}

void Render()
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	for (Draw draw : draws) {
		draw.shader->use();
		int id = draw.shader->id;
		BindBuffers(draw.mesh, glGetAttribLocation(id, "position"), glGetAttribLocation(id, "normals"), glGetAttribLocation(id, "uv"), glGetAttribLocation(id, "tangent"), -1);
		glDrawElements(GL_TRIANGLES, draw.mesh->indices.size(), GL_UNSIGNED_INT, NULL);
	}
	// Draw
	SDL_GL_SwapWindow(window);
}

void ProcessEvent(SDL_Event* e)
{
	switch (e->type) {
	case SDL_MOUSEMOTION:
		//camera->MouseCallback(e);
		break;
	case SDL_MOUSEBUTTONDOWN:
	case SDL_MOUSEBUTTONUP:
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
		SDL_Keymod mod;
		mod = SDL_GetModState();
		if (k == SDLK_ESCAPE)
			quit = 1;

		//if (s == SDL_SCANCODE_D)   active_control_set(0);
		//if (s == SDL_SCANCODE_A)   active_control_set(1);
		//if (s == SDL_SCANCODE_W)   active_control_set(2);
		//if (s == SDL_SCANCODE_S)   active_control_set(3);
		//if (k == SDLK_SPACE)       active_control_set(4);
		//if (s == SDL_SCANCODE_LCTRL)   active_control_set(5);
		//if (s == SDL_SCANCODE_S)   active_control_set(6);
		//if (s == SDL_SCANCODE_D)   active_control_set(7);
		//if (k == '1') global_hack = !global_hack;
		//if (k == '2') global_hack = -1;
		break;
	}
	case SDL_KEYUP: {
		//    if (s == SDL_SCANCODE_D)   active_control_clear(0);
		//    if (s == SDL_SCANCODE_A)   active_control_clear(1);
		//    if (s == SDL_SCANCODE_W)   active_control_clear(2);
		//    if (s == SDL_SCANCODE_S)   active_control_clear(3);
		//    if (k == SDLK_SPACE)       active_control_clear(4);
		//    if (s == SDL_SCANCODE_LCTRL)   active_control_clear(5);
		//    if (s == SDL_SCANCODE_S)   active_control_clear(6);
		//    if (s == SDL_SCANCODE_D)   active_control_clear(7);
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

void InitGame() {
	camera = new Camera(60.0f, SCR_WIDTH, SCR_HEIGHT, 0.1f, 100.0f, vec3(0, 0, -1), 0.0f, 0.0f);
	// meshes.push_back(GetPlane());
	Mesh* mesh = GetPlane();
	InitBuffers(mesh);
	draws.push_back(Draw{ mesh, new Shader{ GetShaderPath("debug_vs.glsl"), GetShaderPath("debug_fs.glsl") } });
}

int SDL_main(int argc, char** argv)
{
	InitContext();
	InitGame();
	while (!quit) {
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			ProcessEvent(&e);
		}
		Render();
	}
	ClearContext();
	return 0;
}
