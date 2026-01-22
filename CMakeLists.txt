# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required(VERSION 3.10...4.0)

project ("sausage")

set (CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR})
include(SausageCmakeUtilities)

add_subdirectory(engine)

add_subdirectory(test)

add_subdirectory(game1)
