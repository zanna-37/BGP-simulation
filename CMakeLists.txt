cmake_minimum_required(VERSION 3.13)
include(ExternalProject)

IF (NOT DEFINED TARGET_EXEC)
    set(TARGET_EXEC BGP_simulation)
ENDIF ()

project(${TARGET_EXEC})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(SRC src)
set(SRC_ENTITIES ${SRC}/entities)


set(SOURCE_FILES
        ${SRC}/main.cpp)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})


###### Libraries and dependencies #####

set(LIBRARIES_FOLDER lib)


# PcapPlusPlus
set(PCAP_PP_ROOT ${LIBRARIES_FOLDER}/PcapPlusPlus)
set(PCAP_PP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/PcapPlusPlus-install-dir)
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PCAP_PP_ROOT}/configure-linux.sh")
    ExternalProject_Add(PcapPlusPlus
            SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${PCAP_PP_ROOT}
            BUILD_IN_SOURCE true
            CONFIGURE_COMMAND mkdir -p ${PCAP_PP_INSTALL_DIR} && chmod +x ./configure-linux.sh && ./configure-linux.sh --default --install-dir ${PCAP_PP_INSTALL_DIR}
            BUILD_COMMAND make libs
            INSTALL_COMMAND make install
            PREFIX ${PCAP_PP_ROOT}
            )

    add_dependencies(${CMAKE_PROJECT_NAME} PcapPlusPlus)
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${PCAP_PP_INSTALL_DIR}/include/pcapplusplus)
    target_link_libraries(${CMAKE_PROJECT_NAME}
            ${PCAP_PP_INSTALL_DIR}/lib/libPacket++.a
            ${PCAP_PP_INSTALL_DIR}/lib/libCommon++.a
            # ${PCAP_PP_INSTALL_DIR}/lib/libPcap++.a # NOT NEEDED
            )
ELSE ()
    MESSAGE(FATAL_ERROR "PcapPlusPlus git submodule is absent. Run `git submodule update --init --recursive` to get it")
ENDIF ()
