#ifndef TCPSTATEFINWAIT1_H
#define TCPSTATEFINWAIT1_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateFINWait1 : public TCPState {
   private:
   public:
    TCPStateFINWait1(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "FIN-WAIT-1";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateFINWait1(){};

    bool onEvent(TCPEvent);
};
#endif