#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <IPv4Layer.h>
#include <TcpLayer.h>

#include "../entities/Device.h"
#include "TCPEvent.h"
#include "fsm/TCPState.h"
#include "fsm/TCPStateMachine.h"

// forward declarations
#include "../entities/Device.fwd.h"
#include "fsm/TCPState.fwd.h"
#include "fsm/TCPStateMachine.fwd.h"

class TCPConnection {
   public:
    TCPStateMachine* stateMachine = nullptr;

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