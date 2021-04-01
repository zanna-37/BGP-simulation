#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <stack>

#include "../../logger/Logger.h"
#include "../TCPEvent.h"
#include "TCPStateMachine.h"

class TCPStateMachine;  // forward declaration
class TCPState {
   public:
    TCPStateMachine* stateMachine;
    std::string      NAME;

    TCPState(TCPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~TCPState() {}

    virtual bool onEvent(TCPEvent) = 0;
};
#endif
