#ifndef TCPSTATESYNSENT_H
#define TCPSTATESYNSENT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateEnstablished.h"
// #include "TCPStateMachine.h"
#include "TCPStateSYNReceived.h"
class TCPStateSYNSent : public TCPState {
   public:
    TCPStateSYNSent(
        TCPStateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine)
        : TCPState(stateMachine) {
        NAME = "SYN-SENT";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateSYNSent(){};

    bool onEvent(TCPEvent);
};
#endif