#ifndef TCPSTATE_H
#define TCPSTATE_H


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
};
#endif
