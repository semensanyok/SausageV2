cmake_minimum_required (VERSION 3.8)

project ("game1")

add_executable(${PROJECT_NAME} "src/game.cpp" "src/game.h" "src/Scene.h")

target_link_libraries(${PROJECT_NAME} PUBLIC engine)

FILE(COPY "assets/" DESTINATION "${CMAKE_BINARY_DIR}/assets")
