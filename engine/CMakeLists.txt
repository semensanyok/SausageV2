cmake_minimum_required (VERSION 3.8)

project ("engine")

set (CMAKE_CXX_STANDARD 20)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(ENGINE_DEP_DIR ${CMAKE_CURRENT_LIST_DIR}/dependencies)
set(ENGINE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
############### DEPS START ############################################

# force lib build, not dll
set(BUILD_SHARED_LIBS false)

# ASSIMP
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory("${ENGINE_DEP_DIR}/assimp")

# GLM
add_subdirectory("${ENGINE_DEP_DIR}/glm")

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

# ZLIB
# add_subdirectory("dependencies/zlib")

add_subdirectory("${ENGINE_LIB_DIR}/glad")

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
list(APPEND CMAKE_MODULE_PATH ${ENGINE_LIB_DIR}/SDL)
set(SDL2_PATH ${SDL2_DIR})
# set(LIBROCKET_INCLUDE_DIRS "dependencies/libRocket/Include")


find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${ENGINE_LIB_DIR}/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(freetype REQUIRED)

set(DEPENDENCIES_INCLUDE_DIRS 
                    "${ENGINE_DEP_DIR}/assimp/include/"
                    "${ENGINE_DEP_DIR}/bullet3/src/"
                    "${ENGINE_DEP_DIR}/glm/"
                    "${ENGINE_LIB_DIR}/glad/include"
                    "${ENGINE_DEP_DIR}/assimp/include"
                    "${ENGINE_DEP_DIR}/freetype/include"
                    "${SDL2_INCLUDE_DIR}"
                    #"${FREETYPE_INCLUDE_DIRS}/"
                    "${ENGINE_DEP_DIR}/imgui/"
                    "${ENGINE_DEP_DIR}/imgui/backends/")
############### DEPS END ############################################

####### IMGUI ##################
set(IMGUI_SOURCES
    "${ENGINE_DEP_DIR}/imgui/imgui.cpp"
    "${ENGINE_DEP_DIR}/imgui/imgui_demo.cpp"
    "${ENGINE_DEP_DIR}/imgui/imgui_draw.cpp"
    "${ENGINE_DEP_DIR}/imgui/imgui_widgets.cpp"
    "${ENGINE_DEP_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${ENGINE_DEP_DIR}/imgui/backends/imgui_impl_sdl.cpp"
    "${ENGINE_DEP_DIR}/imgui/imgui_tables.cpp"
)
set(IMGUI_HEADERS
    "${ENGINE_DEP_DIR}/imgui/imconfig.h"
    "${ENGINE_DEP_DIR}/imgui/imgui.h"
    "${ENGINE_DEP_DIR}/imgui/imgui_internal.h"
    "${ENGINE_DEP_DIR}/imgui/imstb_rectpack.h"
    "${ENGINE_DEP_DIR}/imgui/imstb_textedit.h"
    "${ENGINE_DEP_DIR}/imgui/imstb_truetype.h"
    "${ENGINE_DEP_DIR}/imgui/backends/imgui_impl_opengl3.h"
    "${ENGINE_DEP_DIR}/imgui/backends/imgui_impl_sdl.h"
)
####### IMGUI ##################

include_directories("${DEPENDENCIES_INCLUDE_DIRS}")

set(${PROJECT_NAME}_INCLUDE_DIRS 
"${ENGINE_SRC_DIR}/" 
"${ENGINE_SRC_DIR}/systems/" 
"${ENGINE_SRC_DIR}/systems/controller/" 
"${ENGINE_SRC_DIR}/systems/physics/" 
"${ENGINE_SRC_DIR}/utils/" 
"${ENGINE_SRC_DIR}/buffer/" 
"${ENGINE_SRC_DIR}/buffer/consumers/" 
"${ENGINE_SRC_DIR}/gui/"
${IMGUI_HEADERS} 
${DEPENDENCIES_INCLUDE_DIRS})

set(SAUSAGE_LIBS assimp glad glm SDL2::Main 
Bullet3Common BulletCollision BulletDynamics Bullet3Geometry Bullet3Collision Bullet3Dynamics LinearMath
${SDL2_IMAGE_LIBRARIES} ${Freetype}
CACHE INTERNAL "${PROJECT_NAME} libraries" FORCE)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES} 
"src/systems/Camera.cpp" "src/OpenGLHelpers.cpp" "src/Logging.cpp" 
"src/FileWatcher.cpp" "src/Shader.cpp" "src/Texture.cpp" "src/systems/Animation.cpp" "src/systems/AsyncTaskManager.cpp" 
"src/buffer/BufferStorage.cpp" "src/systems/Renderer.cpp" "src/systems/SystemsManager.cpp" 
"src/buffer/consumers/BulletDebugDrawerBufferConsumer.cpp" "src/systems/physics/BulletDebugDrawer.cpp" 
"src/buffer/consumers/MeshDataBufferConsumer.cpp" 
"src/buffer/BufferManager.cpp" 
"src/systems/controller/MouseKeyboardEventProcessor.cpp" 
"src/systems/TextureManager.cpp" 
"src/utils/AnimHelpers.cpp" 
"src/utils/GLMHelpers.cpp" 
"src/utils/ThreadSafeQueue.cpp" 
"src/systems/MeshManager.cpp" 
"src/buffer/consumers/BufferConsumer.cpp"  
"src/systems/RendererContextManager.cpp" 
"src/buffer/consumers/FontBufferConsumerUI.cpp" 
"src/buffer/consumers/OverlayBufferConsumer3D.cpp")

target_link_libraries(${PROJECT_NAME} ${SAUSAGE_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})

target_compile_options(assimp PRIVATE /w)

###### ENGINE OPTIONS ########################
option(SAUSAGE_PROFILE_ENABLE "Enable profiling info" ON)
option(SAUSAGE_DEBUG_DRAW_PHYSICS "Debug draw physics world" ON)
option(SAUSAGE_DEBUG_BUILD "Enable assertions and other debug utilities" ON)
IF ( SAUSAGE_PROFILE_ENABLE )
  add_definitions(-DSAUSAGE_PROFILE_ENABLE)
endif()
IF ( SAUSAGE_DEBUG_DRAW_PHYSICS )
  add_definitions(-DSAUSAGE_DEBUG_DRAW_PHYSICS)
endif()
IF ( SAUSAGE_DEBUG_BUILD )
  add_definitions(-DSAUSAGE_DEBUG_BUILD)
endif()
###### ENGINE OPTIONS END ####################

FILE(COPY "src/shaders/" DESTINATION "${CMAKE_BINARY_DIR}/shaders")
