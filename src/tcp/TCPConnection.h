#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <IPv4Layer.h>
#include <TcpLayer.h>

#include <stack>

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

    Device*            owner   = nullptr;
    pcpp::IPv4Address* srcAddr = nullptr;
    uint16_t           srcPort = 0;
    pcpp::IPv4Address* dstAddr = nullptr;
    uint16_t           dstPort = 0;

    uint16_t BGPPort = 179;

    TCPConnection(Device* owner);
    ~TCPConnection();

    void    enqueueEvent(TCPEvent);
    uint8_t parseTCPFlags(pcpp::tcphdr* tcpHeader);
    void    processMessage(std::stack<pcpp::Layer*>* layers);


    TCPState* getCurrentState();
};

#endif