#ifndef TCPSTATESYNRECEIVED_H
#define TCPSTATESYNRECEIVED_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateEnstablished.h"
#include "TCPStateMachine.h"
class TCPStateSYNReceived : public TCPState {
   public:
    TCPStateSYNReceived(TCPStateMachine* stateMachine)
        : TCPState(stateMachine) {
        NAME = "SYN-RECEIVED";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateSYNReceived(){};

    bool onEvent(TCPEvent);
};
#endif