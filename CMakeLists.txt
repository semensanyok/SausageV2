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
add_subdirectory("dependencies/zlib")


# BULLET
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
set(LIBROCKET_INCLUDE_DIRS "dependencies/libRocket/Include")
set(IMGUI_INC_PATH "dependencies/imgui/" "dependencies/imgui/backends/")

include_directories(dependencies/assimp/include/
                    dependencies/zlib/
					dependencies/assimp/include/
                    dependencies/bullet/src/
                    dependencies/glad/include/
                    dependencies/glfw/include/
                    dependencies/glm/
                    # dependencies/stb/
                    ${SDL2_DIR}/include/
                    ${FREETYPE_INCLUDE_DIRS}/
                    ${LIBROCKET_INCLUDE_DIRS}/
                    ${IMGUI_INC_PATH}
                    )

find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

find_library(Freetype
    NAMES freetype.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/freetype/lib/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

find_library(LibRocketControls2
    NAMES RocketControls.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )
find_library(LibRocketCore2
    NAMES RocketCore.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )
find_library(LibRocketDebugger2
    NAMES RocketDebugger.lib # what to look for
    HINTS "${CMAKE_SOURCE_DIR}/lib/libRocket/" # where to look
    NO_DEFAULT_PATH # do not search system default paths
    )

# check if we found the library
message(STATUS "HELLO_LIB: ${Freetype}")
if (NOT Freetype)
    message(SEND_ERROR "Did not find lib freetype")
endif()

set(FREETYPE_LIBRARY "${Freetype}")

add_subdirectory("dependencies/libRocket/Build")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(freetype REQUIRED)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()

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
# add_library(dearimgui
#     STATIC
#         ${IMGUI_SOURCES}
#         ${IMGUI_HEADERS}
# )
# target_link_libraries(dearimgui glad)
# target_compile_definitions(dearimgui
#     PRIVATE
#         IMGUI_IMPL_OPENGL_LOADER_GLAD=1
# )
####### IMGUI ##################

set(sources ${sources} ${IMGUI_HEADERS} ${IMGUI_SOURCES})

set(sources ${sources} "src/sausage.cpp" "src/Settings.h" "src/sausage.h" "src/Mesh.h" "src/Camera.h" "src/Shader.h" "src/Texture.h" "src/TestShapes.h" "src/AssetUtils.h" "src/Logging.h" "src/gui/RenderInterfaceSDL2.cpp" "src/gui/RenderInterfaceSDL2.h" "src/gui/SystemInterfaceSDL2.cpp" "src/gui/SystemInterfaceSDL2.h" "src/gui/ShellFileInterface.h" "src/gui/ShellFileInterface.cpp"    "src/Logging.cpp" "src/ThreadSafeQueue.h" "src/glcorearb.h" )

#"src/main.cpp" "src/sausage.cpp"
add_executable(${CMAKE_PROJECT_NAME} ${sources} "src/thirdpartytests/main_imgui_test.cpp"  "src/thirdpartytests/sparse_bindless_texarray.h" "src/thirdpartytests/sparse_bindless_texarray.cpp" "src/Structures.h" "src/BufferManager.h")

# RocketCore RocketDebugger RocketControls
target_link_libraries(${CMAKE_PROJECT_NAME} glfw assimp glad glm SDL2::Main zlib ${SDL2_IMAGE_LIBRARIES} ${LibRocketControls2} ${LibRocketDebugger2} ${LibRocketCore2} ${Freetype})

