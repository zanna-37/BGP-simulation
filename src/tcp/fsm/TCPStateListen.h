#ifndef TCPSTATELISTEN_H
#define TCPSTATELISTEN_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateListen : public TCPState {
   public:
    TCPStateListen(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "LISTEN";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateListen(){};

    bool onEvent(TCPEvent);
};
#endif