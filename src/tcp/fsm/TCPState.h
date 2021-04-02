#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <stack>

#include "../../logger/Logger.h"
#include "../../tcp/fsm/TCPStateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"


class TCPStateMachine;
class TCPConnection;
class TCPState {
   public:
    TCPStateMachine* stateMachine;
    std::string      NAME;

    TCPState(TCPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~TCPState() {}

    virtual bool onEvent(TCPEvent) = 0;
};
#endif
