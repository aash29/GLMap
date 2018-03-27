cmake_minimum_required(VERSION 3.8)
project(GLMap)

include_directories(./Include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -g -I./Include -ltess2 -L. -lGLEW -lGL -lglfw -lSOIL")

set(SOURCE_FILES citymap.cpp camera.cpp graphics.cpp imgui.cpp imgui_draw.cpp imgui_impl_glfw_gl3.cpp imgui_demo.cpp appLog.cpp agent.cpp pddltree.cpp)

link_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(GLMap ${SOURCE_FILES})

target_link_libraries(GLMap GLEW; OpenGL; glfw ; SOIL ; libtess2.a)

