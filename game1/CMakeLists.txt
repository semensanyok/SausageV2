cmake_minimum_required (VERSION 3.8)

set(SAUSAGE_GAME_1 "game1")

INCLUDE_DIRECTORIES(
    ${SAUSAGE_INCLUDE_DIRS}
    "../src"
)
message("SAUSAGE_INC_DIRRRRRRRRRR ${SAUSAGE_INCLUDE_DIRS}")
set(sources ${sources} ${SAUSAGE_INCLUDE_DIRS} "src/game.cpp" )

add_executable(${SAUSAGE_GAME_1} ${sources})

target_link_libraries(game1 engine)

file(GLOB files "src/assets/*")
foreach(file ${files})
  file(RELATIVE_PATH filename "${CMAKE_CURRENT_LIST_DIR}/src/" ${file})
  message(${filename})
  FILE(COPY "${file}/" DESTINATION "${CMAKE_BINARY_DIR}/${filename}")
endforeach()
