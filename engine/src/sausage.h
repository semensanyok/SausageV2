#pragma once
// Engine header file, common imports
// standart and external lib imports.

//#include <SDL.h>
#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "glm/gtx/string_cast.hpp"

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/mesh.h>
#include <assimp/postprocess.h>

#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>

#include <optional>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <queue>
#include <string>
#include <set>
#include <unordered_set>
#include <stack>
#include <tuple>

#include <algorithm>
#include <numeric>
#include <chrono>
#include <functional>
#include <regex>
#include <format>

#include <memory>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <source_location>
#include <typeinfo>

#include <thread>
#include <mutex>
#include <shared_mutex>

#include <stdint.h>

#include <stdio.h>

// disables calls to 
//#define NDEBUG
//#include <assert.h>
#include <cassert>
