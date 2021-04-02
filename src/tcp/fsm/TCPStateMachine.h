#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H

#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

#include "../../entities/Device.h"
#include "../../fsm/StateMachine.h"
#include "../../logger/Logger.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../fsm/TCPStateClosed.h"

class TCPState;       // forward declaration
class TCPConnection;  // forward declaration
template <typename Connection, typename State, typename Event>
class TCPStateMachine : public StateMachine<Connection, State, Event> {
   public:
    TCPStateMachine(Connection* connection)
        : StateMachine<Connection, State, Event>(connection) {
        this->currentState = new TCPStateClosed(this);
    }
};
#endif