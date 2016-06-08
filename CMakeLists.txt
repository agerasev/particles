cmake_minimum_required(VERSION 2.6)
project(particles)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fpermissive")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_C_FLAGS} -std=c++11 -fopenmp")

# add_definitions(-DCL_NO_GL_INTEROP)

set(SOURCE "")

if(${WIN32})
	include_directories(${SDL2_INCLUDE_DIR})
	link_directories(${SDL2_LINK_DIR})
	
	include_directories(${GLEW_INCLUDE_DIR})
	link_directories(${GLEW_LINK_DIR})
	
	include_directories(${OPENCL_INCLUDE_DIR})
	link_directories(${OPENCL_LINK_DIR})
endif()

include_directories(libla)
include_directories(source)
include_directories(kernels)

add_subdirectory(libgl4u)
include_directories(libgl4u/includes)

add_subdirectory(libcl4u)
include_directories(libcl4u/include)

add_executable(particles source/main.cpp ${SOURCE})

if(${WIN32})
	set(LIBS mingw32 SDL2main SDL2 opengl32 glew32 OpenCL)
else()
	set(LIBS SDL2 GL GLEW OpenCL)
endif()

set(LIBS ${LIBS} gl4u cl4u)

target_link_libraries(particles ${LIBS})
