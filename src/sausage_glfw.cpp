// sausage.cpp : Defines the entry point for the application.
//

#include "sausage.h"
#include "Settings.h"

using namespace std;
using namespace glm;

class SceneNode {
	vec3 position;
	vector<SceneNode> children;
	virtual void Draw() = 0;
};

GLFWwindow* InitContext() {
	if (!glfwInit()) {
		std::cerr << "glfwInit() initialization failed" << std::endl;
		return nullptr;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwSetErrorCallback([](int error, const char* description) {
		std::cerr << "Error: " << description << std::endl;
	});
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "My Title", NULL, NULL);
	if (!window) {
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return window;
	}
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
	});
	return window;
}

void Render(GLFWwindow* window)
{
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	// Draw
	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main()
{
	GLFWwindow* window = InitContext();
	double last_frame = glfwGetTime();
	while (!glfwWindowShouldClose(window))
	{
		double current_frame = glfwGetTime();
		double delta_time = current_frame - last_frame;
		last_frame = current_frame;
		Render(window);
	}
	return 0;
}
