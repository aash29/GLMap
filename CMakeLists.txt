cmake_minimum_required(VERSION 3.8)
project(GLMap)


set (CMAKE_CXX_STANDARD 11)
file(GLOB_RECURSE SOURCES RELATIVE ${CMAKE_SOURCE_DIR} "Box2D/*.cpp")

include_directories(./Include)

include_directories(${CMAKE_SOURCE_DIR})
add_library(Box2Dlocal SHARED ${SOURCES})



set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11  -g -I./Include -L. -lGLEW -lGL -lglfw -lSOIL")

set(SOURCE_FILES citymap.cpp imgui.cpp imgui_draw.cpp imgui_impl_glfw_gl3.cpp imgui_demo.cpp appLog.cpp agent.cpp tinyxml2.cpp Include/map.hpp DebugDraw.cpp RenderGL3.cpp Include/agent.h entity.cpp entity.h)

link_directories(${CMAKE_SOURCE_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(GLMap ${SOURCE_FILES} entity.cpp entity.h)

target_link_libraries(GLMap GLEW; OpenGL; glfw ; SOIL ; libtess2.a; Box2Dlocal)
