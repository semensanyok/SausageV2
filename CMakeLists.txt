# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("sausage")

set (CMAKE_CXX_STANDARD 20)
# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory("dependencies/glfw")

# ASSIMP
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory("dependencies/assimp")

# GLAD
add_subdirectory("dependencies/glad")

# GLM
add_subdirectory("dependencies/glm")

# ZLIB
# add_subdirectory("dependencies/zlib")


# BULLET
option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)
add_subdirectory(dependencies/bullet3)

set(SDL2_DIR "lib/SDL")
# set(SDL2_IMAGE_DIR "lib/SDL_image")
# set(SDL2_IMAGE_LIBRARIES "${SDL2_IMAGE_DIR}/SDL2_image.lib")
# set(SDL2_IMAGE_INCLUDE_DIR ${SDL2_IMAGE_DIR}/include/)

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/SDL)
set(SDL2_PATH ${SDL2_DIR})

include_directories(dependencies/assimp/include/
                    # dependencies/zlib/
					dependencies/assimp/include/
                    dependencies/bullet/src/
                    dependencies/glad/include/
                    dependencies/glfw/include/
                    dependencies/glm/
                    # dependencies/stb/
                    ${SDL2_DIR}/include/
					)

add_executable(${CMAKE_PROJECT_NAME} "src/sausage.cpp" "src/Settings.h" "src/sausage.h" "src/Mesh.h" "src/Camera.h" "src/Shader.h" "src/Texture.h" "src/TestShapes.h" "src/AssetUtils.h")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
  #add_test(validate_${i}, "validator", ${file})
endforeach()

target_link_libraries(${CMAKE_PROJECT_NAME} glfw assimp glad glm SDL2::Main ${SDL2_IMAGE_LIBRARIES})
