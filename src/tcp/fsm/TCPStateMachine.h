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
class TCPStateMachine : public StateMachine<TCPConnection, TCPState, TCPEvent> {
   public:
    TCPStateMachine(TCPConnection* connection) : StateMachine(connection) {}
};
#endif