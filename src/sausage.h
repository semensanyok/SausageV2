#pragma once
// Engine header file, common imports

// GLFW. uncomment if decide to switch back from SDL
// #define GLFW_INCLUDE_NONE
// #include <GLFW/glfw3.h>


#define GL_GLEXT_PROTOTYPES
#include <SDL.h>
#include <glad/glad.h>
#include "glcorearb.h"

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <optional>
#include <list>
#include <vector>
#include <map>
#include <queue>
#include <algorithm>
#include <string>

#include <memory>
#include <iostream>
#include <fstream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include <stdint.h>