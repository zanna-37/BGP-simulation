cmake_minimum_required(VERSION 3.13)
project(BGP_simulation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SRC src)
set(SRC_ENTITIES ${SRC}/entities)


set(SOURCE_FILES
		${SRC}/main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})


###### Libraries and dependencies #####

set(LIBRARIES_FOLDER lib)

# Tins
set(LIBTINS_ROOT ${LIBRARIES_FOLDER}/libtins)
IF(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${LIBTINS_ROOT}/CMakeLists.txt")
	add_subdirectory(${LIBTINS_ROOT})
	target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${LIBTINS_ROOT}/include)
	target_link_libraries(${CMAKE_PROJECT_NAME} tins)
ELSE()
	MESSAGE(FATAL_ERROR "libtins git submodule is absent. Run `git submodule update --init --recursive` to get it")
ENDIF()
