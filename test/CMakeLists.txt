cmake_minimum_required (VERSION 3.8)

set(SAUSAGE_TEST_PROJ "sausage_test")

INCLUDE_DIRECTORIES(
    ${SAUSAGE_INCLUDE_DIRS}
    "../src"
)
add_executable(${SAUSAGE_TEST_PROJ} "Main.cpp" "utils/Test1.cpp")
target_link_libraries(${SAUSAGE_TEST_PROJ} ${SAUSAGE_LIBS})

file(GLOB test_assets "${CMAKE_CURRENT_LIST_DIR}/test/assets/*")
message("test assets:")
foreach(file ${test_files})
    file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/test/assets" ${file})
    message("${filename}")
    FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()