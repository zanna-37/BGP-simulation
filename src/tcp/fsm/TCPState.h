#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <stack>

#include "../../fsm/StateMachine.h"
#include "../../logger/Logger.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"

template <class Connection, class State, class Event>
class StateMachine;
class TCPConnection;
class TCPState {
   public:
    StateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine;
    std::string                                      NAME;

    TCPState(StateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine)
        : stateMachine(stateMachine){};
    virtual ~TCPState() {}

    virtual bool onEvent(TCPEvent) = 0;
};
#endif
