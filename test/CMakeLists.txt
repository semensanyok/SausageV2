cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)

set (PROJECT_NAME "sausage_test")

include_directories(
	"src/" 
	"utils/"
	"systems/"
)

#### GTK start ######################################################
#set(GRESOURCE_C   testresources.c)
#set(GRECOURCES_GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR})
#set(GRESOURCE_C_LOC   ${GRECOURCES_GENERATED_PATH}/${GRESOURCE_C})
#set(GRESOURCE_RESOURCES_PATH testresources)
#set(GTK_UI_PATH ${CMAKE_CURRENT_LIST_DIR}/assets)
#
#file(GLOB files "${GTK_UI_PATH}/*")
#foreach(file ${files})
#    list(APPEND gresources_deps ${file})
#endforeach()
#
#MIGRATE_GLADE_GTK3_TO_GTK4_WITH_SUFFIX(${GTK_UI_PATH})
#
#add_custom_command(
#    OUTPUT ${GRESOURCE_C}
#    COMMENT "glib-compile-resources ${GRESOURCE_RESOURCES_PATH}"
#    WORKING_DIRECTORY ${GTK_UI_PATH}
#    COMMAND glib-compile-resources
#    ARGS
#        --target=${GRESOURCE_C_LOC}
#        ${GRESOURCE_RESOURCES_PATH}
#        --generate-source
#    DEPENDS ${gresources_deps}
#)
#
#set_source_files_properties(
#    ${GRESOURCE_C_LOC}
#    PROPERTIES GENERATED TRUE
#)
#### GTK end ######################################################

add_executable(${PROJECT_NAME} "Main.cpp"
"systems/UIFontManagerTest.h"
"systems/RendererContextManagerTest.h"
"systems/3DFontManagerTest.h"
"systems/GtkGLOverlayTest.h"
"systems/GTKExampleApps.h"
${GRESOURCE_C_LOC}
 "systems/ScreenOverlayManagerTest.h" "systems/SausageTestBase.h" "systems/world_gen/TerrainTest.h")
#add_dependencies(${PROJECT_NAME} dummy-resource)
target_link_libraries(${PROJECT_NAME} engine)
target_include_directories(${PROJECT_NAME} PUBLIC ${GRECOURCES_GENERATED_PATH})
#add_dependencies(${SAUSAGE_TEST_PROJ} ${TEST_GRESOURCE})
