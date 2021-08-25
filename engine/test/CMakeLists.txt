cmake_minimum_required (VERSION 3.8)

set(SAUSAGE_TEST_PROJ "sausage_test")

message("AAAAAAAA ${${PROJECT_NAME}_INCLUDE_DIRS}")
# ${PROJECT_NAME} == sausage
INCLUDE_DIRECTORIES(
    "${${PROJECT_NAME}_INCLUDE_DIRS}"
)
add_executable(${SAUSAGE_TEST_PROJ} "Main.cpp" "utils/Test1.h" "utils/GLMHelpersTest.h")
