#ifndef TCPSTATEENSTABLISHED_H
#define TCPSTATEENSTABLISHED_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateCloseWait.h"
#include "TCPStateFINWait1.h"
// #include "TCPStateMachine.h"
class TCPStateEnstablished : public TCPState {
   public:
    TCPStateEnstablished(TCPStateMachine* stateMachine)
        : TCPState(stateMachine) {
        NAME = "ENSTABLISHED";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateEnstablished(){};

    bool onEvent(TCPEvent);
};
#endif