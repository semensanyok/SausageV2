cmake_minimum_required (VERSION 3.8)

project ("engine")

set (CMAKE_CXX_STANDARD 20)

set(ENGINE_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(ENGINE_DEP_DIR ${CMAKE_CURRENT_LIST_DIR}/dependencies)
set(ENGINE_LIB_DIR ${CMAKE_CURRENT_LIST_DIR}/lib)
############### DEPS START ############################################

# force lib build, not dll
set(BUILD_SHARED_LIBS OFF)

# ASSIMP
option(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
option(ASSIMP_BUILD_SAMPLES OFF)
option(ASSIMP_BUILD_TESTS OFF)
add_subdirectory("${ENGINE_DEP_DIR}/assimp")

target_compile_options(assimp PRIVATE /w)

#find_library(assimp
#    NAMES assimp-vc142-mtd.lib # what to look for
#    HINTS "${ENGINE_LIB_DIR}/assimp/" # where to look
#    NO_DEFAULT_PATH # do not search system default paths
#    REQUIRED
#)

# GLM
add_subdirectory("${ENGINE_DEP_DIR}/glm")

add_definitions(-DIMGUI_IMPL_OPENGL_LOADER_GLAD)

add_subdirectory("${ENGINE_LIB_DIR}/glad")

add_subdirectory("${ENGINE_DEP_DIR}/FastNoise2")

# BULLET
# multithread support. something about defining BT_THREADSAFE 
# option(BULLET2_USE_THREAD_LOCKS ON)

option(BUILD_BULLET2_DEMOS OFF)
option(BUILD_CPU_DEMOS OFF)
option(BUILD_EXTRAS OFF)
option(BUILD_OPENGL3_DEMOS OFF)
option(BUILD_UNIT_TESTS OFF)

set(BUILD_SHARED_LIBS_SAVED "${BUILD_SHARED_LIBS}")
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(dependencies/bullet3)
set(BUILD_SHARED_LIBS "${BUILD_SHARED_LIBS_SAVED}")

# Add SDL2 CMake modules
list(APPEND CMAKE_MODULE_PATH ${ENGINE_LIB_DIR}/SDL)
set(SDL2_PATH ${SDL2_DIR})

find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${ENGINE_LIB_DIR}/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(freetype REQUIRED)

# LIBRARIES FOUND BY PKG CONFIG ############################
find_package(PkgConfig REQUIRED)

get_filename_component(PARENT_DIR ${CMAKE_CURRENT_LIST_DIR} DIRECTORY)
set(ENV{PKG_CONFIG_PATH} "$ENV{PKG_CONFIG_PATH};${ENGINE_LIB_DIR}/harfbuzz/lib/pkgconfig")
pkg_check_modules(harfbuzz REQUIRED harfbuzz)
set(PGK_CONF_FOUND_LIBS ${harfbuzz_LIBRARIES})
set(PGK_CONF_FOUND_LIBS_INCLUDE_DIRS ${harfbuzz_INCLUDE_DIRS})
set(PGK_CONF_FOUND_LIBS_LIBRARY_DIRS ${harfbuzz_LIBRARY_DIRS})

#pkg_check_modules(GTK4 REQUIRED gtk4)
#set(PGK_CONF_FOUND_LIBS ${GTK4_LIBRARIES})
#set(PGK_CONF_FOUND_LIBS_INCLUDE_DIRS ${GTK4_INCLUDE_DIRS})
#set(PGK_CONF_FOUND_LIBS_LIBRARY_DIRS ${GTK4_LIBRARY_DIRS})
# LIBRARIES FOUND BY PKG CONFIG END ############################

###### GTK ##################################################

#set(GRESOURCE_C   gtkresources.c)
#set(GRESOURCE_C_LOC   ${GRECOURCES_GENERATED_PATH}/${GRESOURCE_C})
#set(GRESOURCE_RESOURCES_PATH gtkresources)
#set(GTK_UI_PATH ${ENGINE_SRC_DIR}/systems/gtk/ui)
#set(GRECOURCES_GENERATED_PATH ${GTK_UI_PATH})
#
#file(GLOB files "${GTK_UI_PATH}/*")
#foreach(file ${files})
#    list(APPEND gresources_deps ${file})
#endforeach()
#
#MIGRATE_GLADE_GTK3_TO_GTK4_WITH_SUFFIX(${GTK_UI_PATH})
#
#add_custom_command(
#    OUTPUT ${GRESOURCE_C}
#    COMMENT "glib-compile-resources ${GRESOURCE_RESOURCES_PATH}"
#    WORKING_DIRECTORY ${GTK_UI_PATH}
#    COMMAND glib-compile-resources
#    ARGS
#        --target=${GRESOURCE_C_LOC}
#        ${GRESOURCE_RESOURCES_PATH}
#        --generate-source
#    DEPENDS ${gresources_deps}
#)

###### GTK END ################################################


MY_PRINT_ALL_VARIABLES()
set(DEPENDENCIES_INCLUDE_DIRS 
                    "${ENGINE_DEP_DIR}/assimp/include/"
                    "${ENGINE_LIB_DIR}/assimp/include/"
                    "${ENGINE_DEP_DIR}/bullet3/src/"
                    "${ENGINE_DEP_DIR}/glm/"
                    "${ENGINE_LIB_DIR}/glad/include"
                    "${ENGINE_DEP_DIR}/freetype/include"
                    "${SDL2_INCLUDE_DIR}"
                    "${ENGINE_DEP_DIR}/imgui/"
                    "${ENGINE_DEP_DIR}/imgui/backends/"
                    "${ENGINE_DEP_DIR}/cairo/src"
                    # "${ENGINE_DEP_DIR}/cairo/build/src"
                    "${ENGINE_LIB_DIR}/harfbuzz/include/harfbuzz"
                    "${PGK_CONF_FOUND_LIBS_INCLUDE_DIRS}")
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
####### IMGUI END ##################

include_directories("${DEPENDENCIES_INCLUDE_DIRS}")

set(${PROJECT_NAME}_INCLUDE_DIRS 
"${ENGINE_SRC_DIR}/" 
"${ENGINE_SRC_DIR}/systems/" 
"${ENGINE_SRC_DIR}/systems/controller/" 
"${ENGINE_SRC_DIR}/systems/physics/" 
"${ENGINE_SRC_DIR}/systems/gui/"
"${ENGINE_SRC_DIR}/utils/" 
"${ENGINE_SRC_DIR}/buffer/" 
"${ENGINE_SRC_DIR}/buffer/consumers/" 
"${ENGINE_SRC_DIR}/systems/world_gen/" 
"${ENGINE_SRC_DIR}/structures/" 
${IMGUI_HEADERS} 
${DEPENDENCIES_INCLUDE_DIRS})

set(SAUSAGE_LIBS 
assimp 
glad 
glm 
FastNoise
SDL2::Main 
Bullet3Common 
BulletCollision 
BulletDynamics 
Bullet3Geometry 
Bullet3Collision 
Bullet3Dynamics 
LinearMath
${SDL2_IMAGE_LIBRARIES} 
${Freetype} 
${PGK_CONF_FOUND_LIBS} 
CACHE INTERNAL "${PROJECT_NAME} libraries" FORCE)

add_library(${PROJECT_NAME} STATIC ${IMGUI_SOURCES} 
"src/systems/Camera.cpp" "src/utils/OpenGLHelpers.cpp" "src/Logging.cpp" 
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
"src/buffer/consumers/UIBufferConsumer.cpp" 
"src/buffer/consumers/OverlayBufferConsumer3D.cpp" 
"src/systems/gui/ScreenOverlayManager.cpp"
"src/systems/ShaderManager.cpp"
"src/systems/world_gen/TerrainManager.cpp"
${source} "src/structures/Interfaces.h" "src/utils/NumberPool.cpp" )

target_link_directories(${PROJECT_NAME} PUBLIC ${PGK_CONF_FOUND_LIBS_LIBRARY_DIRS})

target_link_libraries(${PROJECT_NAME} ${SAUSAGE_LIBS})

target_include_directories(${PROJECT_NAME} PUBLIC ${${PROJECT_NAME}_INCLUDE_DIRS})


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

function(POSTPROCESS_SHADERS)
    file(GLOB shaders_files "src/shaders/*")
    list(FILTER shaders_files EXCLUDE REGEX "__\.py")
    foreach(file ${shaders_files})
        get_filename_component(filename ${file} NAME)
        list(APPEND out_shaders_files ${CMAKE_BINARY_DIR}/shaders/${filename})
    endforeach()
    add_custom_target(
        dummy_target_generate_shaders ALL
        DEPENDS ${out_shaders_files}
    )
    add_custom_command(
        OUTPUT ${out_shaders_files}
        COMMENT "GENERATE SHADERS"
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/scripts
        COMMAND python
        ARGS
            -m
            generate_shaders
            ${ENGINE_SRC_DIR}
            ${CMAKE_BINARY_DIR}
        DEPENDS ${shaders_files}
    )
endfunction()
POSTPROCESS_SHADERS()

#FILE(COPY "src/shaders/" DESTINATION "${CMAKE_BINARY_DIR}/shaders")

FILE(COPY "dlls/" DESTINATION "${CMAKE_BINARY_DIR}")
