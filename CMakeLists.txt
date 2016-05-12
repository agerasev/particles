cmake_minimum_required(VERSION 2.6)
project(voxie)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11 -Wall -fpermissive")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall")

set(SOURCE "")

include_directories(libla)
include_directories(source)

add_subdirectory(libgl4u)
include_directories(libgl4u/includes)

if(${WIN32})
	include_directories(${SDL2_INCLUDE_DIR})
	include_directories(${GLEW_INCLUDE_DIR})
	link_directories(${SDL2_LINK_DIR})
	link_directories(${GLEW_LINK_DIR})
endif()

add_executable(main source/main.cpp ${SOURCE})

if(${WIN32})
	set(LIBS mingw32 SDL2main SDL2 opengl32 glew32)
else()
	set(LIBS SDL2 GL GLEW)
endif()

set(LIBS ${LIBS} gl4u)

target_link_libraries(main ${LIBS})
