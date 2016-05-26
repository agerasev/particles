cmake_minimum_required(VERSION 2.6)
project(particles)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11 -fopenmp")

add_definitions(-DCL_GL_INTEROP)

set(SOURCE "")

include_directories(libla)
include_directories(source)
include_directories(kernels)

add_subdirectory(libgl4u)
include_directories(libgl4u/includes)

add_subdirectory(libcl4u)
include_directories(libcl4u/include)

add_executable(main source/main.cpp ${SOURCE})

set(LIBS SDL2 GL GLEW OpenCL)

set(LIBS ${LIBS} gl4u cl4u)

target_link_libraries(main ${LIBS})
