#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../TCPEvent.h"
#include "TCPStateMachine.h"

// forward declarations
#include "TCPStateMachine.fwd.h"

class TCPState {
   public:
    TCPStateMachine* stateMachine;
    std::string      NAME;

    TCPState(TCPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~TCPState() {}

    virtual bool onEvent(TCPEvent) = 0;

    std::stack<pcpp::Layer*>* craftTCPLayer(uint16_t srcPort,
                                            uint16_t dstPort,
                                            int      flags);
};
#endif
