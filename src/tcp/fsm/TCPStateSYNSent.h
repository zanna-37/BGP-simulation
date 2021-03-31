#ifndef TCPSTATESYNSENT_H
#define TCPSTATESYNSENT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateSYNSent : public TCPState {
   public:
    TCPStateSYNSent(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "SYN-SENT";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateSYNSent(){};

    bool onEvent(TCPEvent);
};
#endif