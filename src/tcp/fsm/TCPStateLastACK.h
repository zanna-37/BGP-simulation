#ifndef TCPSTATELASTACK_H
#define TCPSTATELASTACK_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateClosed.h"
#include "TCPStateMachine.h"
class TCPStateLastACK : public TCPState {
   public:
    TCPStateLastACK(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "LAST-ACK";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateLastACK(){};

    bool onEvent(TCPEvent);
};
#endif