cmake_minimum_required(VERSION 3.6)
project(MVC_UI_test)

set(CMAKE_CXX_STANDARD 14)

set(ELFW_FILES elfw.h elfw-draw.h elfw-base.h)
set(SOURCE_FILES main.cpp elfw-hashing.h elfw-viewtree.h elfw-orderedset.h elfw-debuging.h elfw-diffing.h)
add_executable(MVC_UI_test ${SOURCE_FILES} ${ELFW_FILES})


# ==========


add_subdirectory(tools)
add_subdirectory(vendor)


# ==========

find_package(glfw3 REQUIRED)
find_package(OPENGL REQUIRED)
find_package(GLEW)

set(GLAPP_FILES app/glapp-main.cpp app/load_shader.cpp app/load_shader.h)
add_executable(glapp ${GLAPP_FILES} ${ELFW_FILES})

target_include_directories(glapp
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${GLEW_INCLUDE_DIRS})

target_link_libraries(glapp ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} glfw nanovg)

include_directories(mkzbase/variant/include)

#========================

set(SHADER_DATA_FILE ${CMAKE_CURRENT_BINARY_DIR}/shaders.data )
set(SHADER_DATA_FILES ${SHADER_DATA_FILE} ${SHADER_DATA_FILE}.meta)


add_custom_target(
        generate-shader-data
        DEPENDS ${SHADER_DATA_FILES}
)

add_custom_command(
        COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tools/elfw-resources ${SHADER_DATA_FILE} shaders/basic.frag shaders/basic.vert
        DEPENDS elfw-resources
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/app
        OUTPUT ${SHADER_DATA_FILES}
)

add_dependencies(glapp generate-shader-data)

