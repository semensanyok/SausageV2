cmake_minimum_required (VERSION 3.8)

project("game1")

message("AAAAAAPROJECT_NAME ${PROJECT_NAME}")
INCLUDE_DIRECTORIES(
    ${CMAKE_SOURCE_DIR}/engine/src/
    "src/"
)
set(sources ${sources} ${SAUSAGE_INCLUDE_DIRS} "src/game.cpp" )

add_executable(${SAUSAGE_GAME_1} ${sources})

target_link_libraries(${SAUSAGE_GAME_1} engine)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()
