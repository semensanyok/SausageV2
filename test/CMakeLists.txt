cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)

set(SAUSAGE_TEST_PROJ "sausage_test")

include_directories(
	"src/" 
	"utils/"
)

add_executable(${SAUSAGE_TEST_PROJ} "Main.cpp")

target_link_libraries(${SAUSAGE_TEST_PROJ} engine)