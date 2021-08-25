cmake_minimum_required (VERSION 3.8)

project ("engine")

set (CMAKE_CXX_STANDARD 20)

############### DEPS START ############################################

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
# set(LIBROCKET_INCLUDE_DIRS "dependencies/libRocket/Include")


find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "lib/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(freetype REQUIRED)

set(DEPENDENCIES_INCLUDE_DIRS 
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/assimp/include/"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/bullet3/src/"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/glm/"
                    "${SDL2_DIR}/include/"
                    #"${FREETYPE_INCLUDE_DIRS}/"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/"
                    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/")
############### DEPS END ############################################


####### IMGUI ##################
set(IMGUI_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui_demo.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui_draw.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui_widgets.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/imgui_impl_opengl3.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/imgui_impl_sdl.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui_tables.cpp"
)
set(IMGUI_HEADERS
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imconfig.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imgui_internal.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imstb_rectpack.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imstb_textedit.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/imstb_truetype.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/imgui_impl_opengl3.h"
    "${CMAKE_CURRENT_LIST_DIR}/dependencies/imgui/backends/imgui_impl_sdl.h"
)
####### IMGUI ##################

include_directories("${DEPENDENCIES_INCLUDE_DIRS}")

set(sources ${sources} ${IMGUI_HEADERS} ${IMGUI_SOURCES} )

set(${PROJECT_NAME}_INCLUDE_DIRS 
"${CMAKE_CURRENT_LIST_DIR}/src/" 
"${CMAKE_CURRENT_LIST_DIR}/src/systems/"
"${CMAKE_CURRENT_LIST_DIR}/src/utils/" 
"${CMAKE_CURRENT_LIST_DIR}/src/buffer/" 
"${CMAKE_CURRENT_LIST_DIR}/src/gui/"
# Expose deps includes to other subprojects through cache variable.
${DEPENDENCIES_INCLUDE_DIRS}
CACHE INTERNAL "${PROJECT_NAME} include directories" FORCE)

include_directories(${PROJECT_NAME}_INCLUDE_DIRS)

set(sources "src/systems/Camera.cpp" "src/OpenGLHelpers.cpp" "src/Logging.cpp" 
"src/FileWatcher.cpp" "src/Shader.cpp" "src/Texture.cpp" "src/systems/Animation.cpp" "src/systems/AsyncTaskManager.cpp" 
"src/buffer/BufferStorage.cpp" "src/systems/Renderer.cpp" "src/systems/SystemsManager.cpp" 
"src/buffer/consumers/BulletDebugDrawerBufferConsumer.cpp" "src/systems/physics/BulletDebugDrawer.cpp" 
"src/buffer/consumers/MeshDataBufferConsumer.cpp")

set(SAUSAGE_LIBS assimp glad glm SDL2::Main 
Bullet3Common BulletCollision BulletDynamics Bullet3Geometry Bullet3Collision Bullet3Dynamics LinearMath
${SDL2_IMAGE_LIBRARIES} ${Freetype})

add_library(${PROJECT_NAME} STATIC ${sources})

target_link_libraries(${PROJECT_NAME} ${SAUSAGE_LIBS})

###### ENGINE OPTIONS ########################
option(SAUSAGE_PROFILE_ENABLE "Enable profiling info" ON)
option(SAUSAGE_DEBUG_DRAW_PHYSICS "Debug draw physics world" ON)

IF ( SAUSAGE_PROFILE_ENABLE )
  add_definitions(-DSAUSAGE_PROFILE_ENABLE)
endif()
IF ( SAUSAGE_DEBUG_DRAW_PHYSICS )
  add_definitions(-DSAUSAGE_DEBUG_DRAW_PHYSICS)
endif()
###### ENGINE OPTIONS END ####################

add_subdirectory(test)

FILE(COPY "src/shaders/" DESTINATION "${CMAKE_BINARY_DIR}/shaders")
