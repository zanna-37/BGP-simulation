#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <IPv4Layer.h>
#include <TcpLayer.h>

#include "../entities/Device.h"
#include "../fsm/StateMachine.h"
#include "TCPEvent.h"
#include "fsm/TCPStateClosed.h"
#include "fsm/TCPStateMachine.h"

// OLD
// class TCPStateMachine;  // forward declaration
// class Device;           // forward declaration

template <class Connection, class State, class Event>
class TCPStateMachine;
class TCPState;
class TCPConnection {
   public:
    TCPStateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine = nullptr;

    Device*           owner = nullptr;
    pcpp::IPv4Address srcAddr;
    uint16_t          srcPort = 0;
    pcpp::IPv4Address dstAddr;
    uint16_t          dstPort = 0;

    uint16_t BGPPort = 179;

    TCPConnection(Device* owner);
    ~TCPConnection();

    void enqueueEvent(TCPEvent);

    TCPState* getCurrentState();
};

#endif