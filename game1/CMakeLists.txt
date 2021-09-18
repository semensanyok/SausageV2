cmake_minimum_required (VERSION 3.8)

project("game1")

INCLUDE_DIRECTORIES(
    "${engine_INCLUDE_DIRS}"
)
set(sources "src/game.cpp" )

add_executable(${PROJECT_NAME} ${sources})

target_link_libraries(${PROJECT_NAME} engine)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()
