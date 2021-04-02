#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H

#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

// #include "../../entities/Device.h"
#include "../../fsm/StateMachine.h"
#include "../../logger/Logger.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../fsm/TCPState.h"
#include "../fsm/TCPStateClosed.h"

class TCPState;       // forward declaration
class TCPConnection;  // forward declaration
// template <typename Connection = TCPConnection,
//           typename State      = TCPState,
//           typename Event      = TCPEvent>
class TCPStateMachine : public StateMachine<TCPConnection, TCPState, TCPEvent> {
   public:
    TCPStateMachine(TCPConnection* connection) : StateMachine(connection) {
        // this->currentState = new TCPStateClosed(this);
    }
};
#endif