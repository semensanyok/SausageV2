cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)

set (PROJECT_NAME "sausage_test")

include_directories(
	"src/" 
	"utils/"
	"systems/"
)
set(GRESOURCE_C   testresources.c)
set(GRESOURCE_C_LOC   ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C})
set(GRESOURCE_RESOURCES_PATH testresources)

#set(TEST_GRESOURCE test_gresource)
#add_custom_target(
#    ${TEST_GRESOURCE}
#    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
#)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/assets
    COMMAND glib-compile-resources
    ARGS
        --target=${GRESOURCE_C_LOC}
        ${GRESOURCE_RESOURCES_PATH}
        --generate-source
    #DEPENDS ${GRESOURCE_RESOURCES_PATH}
    #VERBATIM
    #BYPRODUCTS ${GRESOURCE_C_LOC}
    #MAIN_DEPENDENCY ${GRESOURCE_RESOURCES_PATH}
)

add_custom_target(
    dummy-resource
    DEPENDS ${GRESOURCE_C_LOC}
)

set_source_files_properties(
    ${GRESOURCE_C_LOC}
    PROPERTIES GENERATED TRUE
)
#FILE(COPY ${CMAKE_CURRENT_LIST_DIR}/assets/${GRESOURCE_RESOURCES_PATH} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

add_executable(${PROJECT_NAME} "Main.cpp"
"systems/UIFontManagerTest.h"
"systems/RendererContextManagerTest.h"
"systems/3DFontManagerTest.h"
"systems/ScreenOverlayTest.h"
"systems/GTKExampleApps.h"
${GRESOURCE_C_LOC}
)
#add_dependencies(${PROJECT_NAME} dummy-resource)
target_link_libraries(${PROJECT_NAME} engine)
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

#add_dependencies(${SAUSAGE_TEST_PROJ} ${TEST_GRESOURCE})
