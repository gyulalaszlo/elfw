cmake_minimum_required(VERSION 3.6)
project(MVC_UI_test)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp elfw.h elfw-draw.h elfw-base.h)
add_executable(MVC_UI_test ${SOURCE_FILES})

include_directories(mkzbase/variant/include)