cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)

set(SAUSAGE_TEST_PROJ "sausage_test")

INCLUDE_DIRECTORIES(
    "${engine_INCLUDE_DIRS}"
)
add_executable(${SAUSAGE_TEST_PROJ} "Main.cpp" "Test1.h" "utils/GLMHelpersTest.h")

target_link_libraries(${SAUSAGE_TEST_PROJ} engine)