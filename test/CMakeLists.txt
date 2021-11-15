cmake_minimum_required (VERSION 3.8)

set (CMAKE_CXX_STANDARD 20)

set (PROJECT_NAME "sausage_test")

include_directories(
	"src/" 
	"utils/"
	"systems/"
)
set(GRESOURCE_C   testresources.c)
set(GRECOURCES_GENERATED_PATH ${CMAKE_CURRENT_BINARY_DIR})
set(GRESOURCE_C_LOC   ${GRECOURCES_GENERATED_PATH}/${GRESOURCE_C})
set(GRESOURCE_RESOURCES_PATH testresources)
set(GTK_UI_PATH ${CMAKE_CURRENT_LIST_DIR}/assets)

file(GLOB files "${GTK_UI_PATH}/*")
foreach(file ${files})
    list(APPEND gresources_deps ${file})
endforeach()

file(GLOB files "${CMAKE_CURRENT_LIST_DIR}/assets/*.glade")
foreach(file ${files})
    list(APPEND gresources_glade_gtk3_deps ${file})
endforeach()

foreach(file ${gresources_glade_gtk3_deps})
    get_filename_component(filename ${file} NAME)
add_custom_target(
    dummy_target_${filename} ALL
    DEPENDS dummy_${filename}
)
add_custom_command(
    OUTPUT dummy_${filename}
    COMMENT "gtk4-builder-tool 3to4 ${filename}"
    WORKING_DIRECTORY ${GTK_UI_PATH}
    COMMAND gtk4-builder-tool
    ARGS
        simplify
        --3to4
        --replace
        ${filename}
    DEPENDS ${file}
)
endforeach()

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    COMMENT "glib-compile-resources ${GRESOURCE_RESOURCES_PATH}"
    WORKING_DIRECTORY ${GTK_UI_PATH}
    COMMAND glib-compile-resources
    ARGS
        --target=${GRESOURCE_C_LOC}
        ${GRESOURCE_RESOURCES_PATH}
        --generate-source
    DEPENDS ${gresources_deps}
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
target_include_directories(${PROJECT_NAME} PUBLIC ${GRECOURCES_GENERATED_PATH})
#add_dependencies(${SAUSAGE_TEST_PROJ} ${TEST_GRESOURCE})
