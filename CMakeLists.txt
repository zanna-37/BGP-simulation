cmake_minimum_required(VERSION 3.13)
project(BGP_simulation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ansi -pedantic -W -Wall -Werror")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./build)

set(SRC src)
set(ENTITIES ${SRC}/entities)

set(INCLUDE_FOLDER include)
#set(INCLUDES
#        ${INCLUDE_FOLDER}/tins/tins.h
#        )

set(LIBRARIES_FOLDER lib)
set(LIBRARIES
        ${CMAKE_SOURCE_DIR}/${LIBRARIES_FOLDER}/libtins.a
        )

#include_directories(${CMAKE_PROJECT_NAME}
#        ${INCLUDE_FOLDER}
#        )
#link_directories(${CMAKE_PROJECT_NAME}
#        ${INCLUDE_FOLDER}
#        ${LIBRARIES_FOLDER}
#        )

set(SOURCE_FILES
        ${SRC}/main.cpp
        )

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})


target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
        ${INCLUDE_FOLDER}
        )
target_link_libraries(${CMAKE_PROJECT_NAME} PUBLIC
        ${LIBRARIES}
        )
