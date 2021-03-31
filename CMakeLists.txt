cmake_minimum_required(VERSION 3.13)
include(ExternalProject)

IF (NOT DEFINED TARGET_EXEC)
    set(TARGET_EXEC BGP_simulation)
ENDIF ()

project(${TARGET_EXEC})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_DEBUG OFF)
IF (USE_DEBUG)
    add_compile_definitions(DEBUG_GUARD)
    MESSAGE(NOTICE "Entering Debug Mode...")
ENDIF()


set(SRC src)
set(SRC_CONFIGURATION ${SRC}/configuration)
set(SRC_CONFIGURATION_PARSER ${SRC_CONFIGURATION}/parser)
set(SRC_ENTITIES ${SRC}/entities)
set(SRC_LOGGER ${SRC}/logger)
set(SRC_BGP ${SRC}/bgp)
set(SRC_FSM ${SRC_BGP}/fsm) 
set(SRC_UTILS ${SRC}/utils)
set(SRC_IP ${SRC}/ip)
set(SRC_TCP ${SRC}/tcp)
set(SRC_TCP_FSM ${SRC_TCP}/fsm)


set(SOURCE_FILES
        ${SRC}/main.cpp
        ${SRC_CONFIGURATION_PARSER}/Parser.cpp
        ${SRC_CONFIGURATION_PARSER}/ParserClient.cpp
        ${SRC_CONFIGURATION_PARSER}/ParserLink.cpp
        ${SRC_CONFIGURATION_PARSER}/ParserNetworkCard.cpp
        ${SRC_CONFIGURATION_PARSER}/ParserRouter.cpp
        ${SRC_ENTITIES}/EndPoint.cpp
        ${SRC_ENTITIES}/Device.cpp
        ${SRC_ENTITIES}/Link.cpp
        ${SRC_ENTITIES}/NetworkCard.cpp
        ${SRC_ENTITIES}/Router.cpp
        ${SRC_LOGGER}/Logger.cpp
        ${SRC_BGP}/BGPConnection.cpp
        ${SRC_BGP}/Event.cpp
        ${SRC_FSM}/BGPStateActive.cpp
        ${SRC_FSM}/BGPStateConnect.cpp
        ${SRC_FSM}/BGPStateEnstablished.cpp
        ${SRC_FSM}/BGPStateIdle.cpp
        ${SRC_FSM}/BGPStateMachine.cpp
        ${SRC_FSM}/BGPStateOpenConfirm.cpp
        ${SRC_FSM}/BGPStateOpenSent.cpp
        ${SRC_UTILS}/Timer.cpp
        ${SRC_IP}/RoutingTable.cpp
        ${SRC_IP}/TableRow.cpp
        ${SRC_UTILS}/MacGenerator.cpp
        ${SRC_TCP}/TCPConnection.cpp
        ${SRC_TCP}/TCPEvent.cpp
        ${SRC_TCP_FSM}/TCPStateClosed.cpp
        ${SRC_TCP_FSM}/TCPStateClosing.cpp
        ${SRC_TCP_FSM}/TCPStateEnstablished.cpp
        ${SRC_TCP_FSM}/TCPStateListen.cpp
        ${SRC_TCP_FSM}/TCPStateMachine.cpp
        ${SRC_TCP_FSM}/TCPStateSYNReceived.cpp
        ${SRC_TCP_FSM}/TCPStateSYNSent.cpp
        ${SRC_TCP_FSM}/TCPStateCloseWait.cpp
        ${SRC_TCP_FSM}/TCPStateFINWait1.cpp
        ${SRC_TCP_FSM}/TCPStateFINWait2.cpp
        ${SRC_TCP_FSM}/TCPStateLastACK.cpp
        ${SRC_TCP_FSM}/TCPStateTimeWait.cpp

)

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_FILES})

###### -lpthread flag support ######
find_package( Threads )
target_link_libraries( ${CMAKE_PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} )

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

# yaml-cpp
set(YAMLCPP_ROOT ${LIBRARIES_FOLDER}/yaml-cpp)
IF (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${YAMLCPP_ROOT}/CMakeLists.txt")
    add_subdirectory(${YAMLCPP_ROOT})
    target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${YAMLCPP_ROOT}/include)
    target_link_libraries(${CMAKE_PROJECT_NAME} yaml-cpp)
ELSE ()
    MESSAGE(FATAL_ERROR "yaml-cpp git submodule is absent. Run `git submodule update --init --recursive` to get it")
ENDIF ()
