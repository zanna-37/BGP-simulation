#ifndef TCPSTATELISTEN_H
#define TCPSTATELISTEN_H

#include "../TCPConnection.h"
#include "TCPState.h"
// #include "TCPStateMachine.h"
#include "TCPStateSYNReceived.h"
class TCPStateListen : public TCPState {
   public:
    TCPStateListen(
        StateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine)
        : TCPState(stateMachine) {
        NAME = "LISTEN";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateListen(){};

    bool onEvent(TCPEvent);
};
#endif