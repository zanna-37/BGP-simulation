#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H

#include <chrono>

#include "../../fsm/StateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../TCPTimer.h"
#include "../fsm/TCPState.h"

// forward declarations
#include "../TCPConnection.fwd.h"
#include "../TCPTimer.fwd.h"
#include "TCPState.fwd.h"

class TCPStateMachine : public StateMachine<TCPConnection, TCPState, TCPEvent> {
   public:
    TCPTimer* timeWaitTimer = nullptr;

    std::chrono::seconds timeWaitTime = 2s;
    TCPStateMachine(TCPConnection* connection);

    ~TCPStateMachine();

    void initializeTimers();

    void resetTimeWaitTimer();
};
#endif