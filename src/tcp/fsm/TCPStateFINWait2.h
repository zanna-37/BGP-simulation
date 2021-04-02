#ifndef TCPSTATEFINWAIT2_H
#define TCPSTATEFINWAIT2_H

#include "../TCPConnection.h"
#include "TCPState.h"
// #include "TCPStateMachine.h"
#include "TCPStateTimeWait.h"
class TCPStateFINWait2 : public TCPState {
   public:
    TCPStateFINWait2(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "FIN-WAIT-2";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateFINWait2(){};

    bool onEvent(TCPEvent);
};
#endif