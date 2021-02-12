# ATTENTION
# This file is currently not in sync with the Makefile and will not compile
#
cmake_minimum_required(VERSION 3.13)  # CMake version check
project(simple_example)               # Create project "simple_example"

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -pedantic -W -Wall -Werror")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./build)

set(SRC ./src/)
set(ENTITIES ./src/entities)

# include_directories(${LIBRARY})

set(
    SOURCE_FILES
    ${SRC}/main.cpp
)

add_executable(output ${SOURCE_FILES})
