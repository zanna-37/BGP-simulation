#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <stack>

#include "../../logger/Logger.h"
#include "../../tcp/fsm/TCPStateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"

template <class Connection, class State, class Event>
class TCPStateMachine;
class TCPConnection;
class TCPState {
   public:
    TCPStateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine;
    std::string                                         NAME;

    TCPState(TCPStateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine)
        : stateMachine(stateMachine){};
    virtual ~TCPState() {}

    virtual bool onEvent(TCPEvent) = 0;
};
#endif
