cmake_minimum_required(VERSION 3.6)
project(MVC_UI_test)

set(CMAKE_CXX_STANDARD 14)

set(ELFW_FILES elfw.h elfw-draw.h elfw-base.h)
set(SOURCE_FILES main.cpp )
add_executable(MVC_UI_test ${SOURCE_FILES} ${ELFW_FILES})


# ==========


add_subdirectory(tools)



# ==========

find_package(glfw3 REQUIRED)
find_package(OPENGL REQUIRED)
find_package(GLEW)

set(GLAPP_FILES app/glapp-main.cpp app/load_shader.cpp app/load_shader.h)
add_executable(glapp ${GLAPP_FILES} ${ELFW_FILES})

target_include_directories(glapp
        PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${GLEW_INCLUDE_DIRS})

target_link_libraries(glapp ${OPENGL_gl_LIBRARY} ${GLEW_LIBRARIES} glfw)

include_directories(mkzbase/variant/include)