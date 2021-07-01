# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("sausage")

set (CMAKE_CXX_STANDARD 20)
# force lib build, not dll
set(BUILD_SHARED_LIBS false)

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# ASSIMP
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory("dependencies/assimp")

# GLM
add_subdirectory("dependencies/glm")

# ZLIB
# add_subdirectory("dependencies/zlib")

add_subdirectory("lib/glad")

# BULLET
# multithread support. something about defining BT_THREADSAFE 
# option(BULLET2_USE_THREAD_LOCKS ON)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)

add_subdirectory(dependencies/bullet3)

set(SDL2_DIR "lib/SDL")

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/SDL)
set(SDL2_PATH ${SDL2_DIR})
set(FREETYPE_INCLUDE_DIRS "lib/freetype/include/freetype2")
# set(LIBROCKET_INCLUDE_DIRS "dependencies/libRocket/Include")
set(IMGUI_INC_PATH "dependencies/imgui/" "dependencies/imgui/backends/")

set (SAUSAGE_INCLUDE_DIRS dependencies/assimp/include/
                    dependencies/bullet3/src/
                    # dependencies/zlib/
                    dependencies/glm/
                    ${SDL2_DIR}/include/
                    ${FREETYPE_INCLUDE_DIRS}/
                    # ${LIBROCKET_INCLUDE_DIRS}/
                    ${IMGUI_INC_PATH}
                    )

include_directories(${SAUSAGE_INCLUDE_DIRS})
find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

# find_library(LibRocketControls2
#     NAMES RocketControls.lib # what to look for
#     HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
#     NO_DEFAULT_PATH # do not search system default paths
#     )
# find_library(LibRocketCore2
#     NAMES RocketCore.lib # what to look for
#     HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
#     NO_DEFAULT_PATH # do not search system default paths
#     )
# find_library(LibRocketDebugger2
#     NAMES RocketDebugger.lib # what to look for
#     HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
#     NO_DEFAULT_PATH # do not search system default paths
#     )
# add_subdirectory("dependencies/libRocket/Build")

set(FREETYPE_LIBRARY "${Freetype}")


find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(freetype REQUIRED)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()
FILE(COPY "src/shaders/" DESTINATION "${CMAKE_BINARY_DIR}/shaders")

####### IMGUI ##################
set(IMGUI_SOURCES
    "dependencies/imgui/imgui.cpp"
    "dependencies/imgui/imgui_demo.cpp"
    "dependencies/imgui/imgui_draw.cpp"
    "dependencies/imgui/imgui_widgets.cpp"
    "dependencies/imgui/backends/imgui_impl_opengl3.cpp"
    "dependencies/imgui/backends/imgui_impl_sdl.cpp"
    "dependencies/imgui/imgui_tables.cpp"
)
set(IMGUI_HEADERS
    "dependencies/imgui/imconfig.h"
    "dependencies/imgui/imgui.h"
    "dependencies/imgui/imgui_internal.h"
    "dependencies/imgui/imstb_rectpack.h"
    "dependencies/imgui/imstb_textedit.h"
    "dependencies/imgui/imstb_truetype.h"
    "dependencies/imgui/backends/imgui_impl_opengl3.h"
    "dependencies/imgui/backends/imgui_impl_sdl.h"
)
####### IMGUI ##################

# libs
set(sources ${sources} ${IMGUI_HEADERS} ${IMGUI_SOURCES} )
# "src/gui/RenderInterfaceSDL2.cpp" "src/gui/RenderInterfaceSDL2.h" "src/gui/SystemInterfaceSDL2.cpp" "src/gui/SystemInterfaceSDL2.h" "src/gui/ShellFileInterface.h" "src/gui/ShellFileInterface.cpp"
set(sources ${sources} "src/systems/Gui.h" )
set(sources ${sources} "src/sausage.cpp" "src/Settings.h" "src/sausage.h" "src/systems/MeshManager.h" "src/Shader.h" "src/Texture.h" "src/TestShapes.h" "src/utils/AssetUtils.h" "src/Logging.h" "src/Logging.cpp" "src/utils/ThreadSafeQueue.h" "src/OpenGLHelpers.cpp"  )
set(sources ${sources} "src/Structures.h" "src/systems/BufferStorage.h" "src/systems/Renderer.h" "src/systems/Camera.h" "src/systems/Gui.h" "src/OpenGLHelpers.h" "src/systems/SystemsManager.h" "src/Scene.h" "src/systems/Controller.h" "src/systems/Physics.h" "src/systems/TextureManager.h"  "src/FileWatcher.h"  "src/systems/BulletDebugDrawer.h" "src/systems/Animation.h" "src/utils/ColladaLoader.h")
#"src/main.cpp" "src/sausage.cpp"
add_executable(${CMAKE_PROJECT_NAME} ${sources} "src/systems/AsyncTaskManager.h")

# RocketCore RocketDebugger RocketControls 
# ${LibRocketControls2} ${LibRocketDebugger2} ${LibRocketCore2}

set(SAUSAGE_LIBS assimp glad glm SDL2::Main 
Bullet3Common BulletCollision BulletDynamics Bullet3Geometry Bullet3Collision Bullet3Dynamics LinearMath
${SDL2_IMAGE_LIBRARIES} ${Freetype})
target_link_libraries(${CMAKE_PROJECT_NAME} ${SAUSAGE_LIBS})

add_subdirectory(test)

###### ENGINE OPTIONS ##########
option(SAUSAGE_PROFILE_ENABLE "Enable profiling info" ON)
option(SAUSAGE_DEBUG_DRAW_PHYSICS "Debug draw physics world" ON)

IF ( SAUSAGE_PROFILE_ENABLE )
  add_definitions(-DSAUSAGE_PROFILE_ENABLE)
endif()
IF ( SAUSAGE_DEBUG_DRAW_PHYSICS )
  add_definitions(-DSAUSAGE_DEBUG_DRAW_PHYSICS)
endif()