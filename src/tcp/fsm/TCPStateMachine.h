#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H


#include "../../fsm/StateMachine.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"
#include "../fsm/TCPState.h"

// forward declarations
#include "../TCPConnection.fwd.h"
#include "TCPState.fwd.h"

class TCPStateMachine : public StateMachine<TCPConnection, TCPState, TCPEvent> {
   public:
    TCPStateMachine(TCPConnection* connection);
};
#endif