cmake_minimum_required (VERSION 3.8)

project ("engine")

set (CMAKE_CXX_STANDARD 20)
# force lib build, not dll
set(BUILD_SHARED_LIBS false)

# ASSIMP
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/assimp")

# GLM
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/dependencies/glm")

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# ZLIB
# add_subdirectory("dependencies/zlib")

add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/lib/glad")

# BULLET
# multithread support. something about defining BT_THREADSAFE 
# option(BULLET2_USE_THREAD_LOCKS ON)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)

add_subdirectory(dependencies/bullet3)

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/SDL)
set(SDL2_PATH ${SDL2_DIR})
set(FREETYPE_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/lib/freetype/include/freetype2")
# set(LIBROCKET_INCLUDE_DIRS "dependencies/libRocket/Include")
set(IMGUI_INC_PATH "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/" "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/")

include_directories(dependencies/assimp/include/
                    dependencies/bullet3/src/
                    # dependencies/zlib/
                    dependencies/glm/
                    ${SDL2_DIR}/include/
                    ${FREETYPE_INCLUDE_DIRS}/
                    # ${LIBROCKET_INCLUDE_DIRS}/
                    ${IMGUI_INC_PATH})
find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/engine/lib/freetype/lib/" # where to look
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

set(sources ${sources} ${IMGUI_HEADERS} ${IMGUI_SOURCES} )
# "src/gui/RenderInterfaceSDL2.cpp" "src/gui/RenderInterfaceSDL2.h" "src/gui/SystemInterfaceSDL2.cpp" "src/gui/SystemInterfaceSDL2.h" "src/gui/ShellFileInterface.h" "src/gui/ShellFileInterface.cpp"

set(SAUSAGE_INCLUDE_DIRS 
"${CMAKE_CURRENT_LIST_DIR}/src/sausage.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/Gui.h"
"${CMAKE_CURRENT_LIST_DIR}/src/Settings.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/MeshManager.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/Shader.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/Texture.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/TestShapes.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/AssetUtils.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/Logging.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/ThreadSafeQueue.h"
"${CMAKE_CURRENT_LIST_DIR}/src/Structures.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/BufferStorage.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/Renderer.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/Camera.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/Gui.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/OpenGLHelpers.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/SystemsManager.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/controller/Controller.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/physics/Physics.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/TextureManager.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/FileWatcher.h"  
"${CMAKE_CURRENT_LIST_DIR}/src/systems/physics/BulletDebugDrawer.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/Animation.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/ColladaLoader.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/AsyncTaskManager.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/StateManager.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/AssimpHelper.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/GLMHelpers.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/utils/AnimHelpers.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/controller/MouseKeyboardEventProcessor.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/MeshDataBufferConsumer.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/BulletDebugDrawerBufferConsumer.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/FontBufferConsumer.h" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/BufferManager.h" 
PARENT_SCOPE)

set(sources ${SAUSAGE_INCLUDE_DIRS} "src/systems/Camera.cpp" "src/OpenGLHelpers.cpp" "src/Logging.cpp" "src/FileWatcher.cpp" "src/Shader.cpp" "src/Texture.cpp" "src/systems/Animation.cpp" "src/systems/AsyncTaskManager.cpp")
add_library(${CMAKE_PROJECT_NAME} ${sources})

set(SAUSAGE_LIBS assimp glad glm SDL2::Main 
Bullet3Common BulletCollision BulletDynamics Bullet3Geometry Bullet3Collision Bullet3Dynamics LinearMath
${SDL2_IMAGE_LIBRARIES} ${Freetype})
target_link_libraries(${CMAKE_PROJECT_NAME} ${SAUSAGE_LIBS})

###### ENGINE OPTIONS ##########
option(SAUSAGE_PROFILE_ENABLE "Enable profiling info" ON)
option(SAUSAGE_DEBUG_DRAW_PHYSICS "Debug draw physics world" ON)

IF ( SAUSAGE_PROFILE_ENABLE )
  add_definitions(-DSAUSAGE_PROFILE_ENABLE)
endif()
IF ( SAUSAGE_DEBUG_DRAW_PHYSICS )
  add_definitions(-DSAUSAGE_DEBUG_DRAW_PHYSICS)
endif()

add_subdirectory(test)

FILE(COPY "src/shaders/" DESTINATION "${CMAKE_BINARY_DIR}/shaders")
