#ifndef TCPSTATECLOSING_H
#define TCPSTATECLOSING_H

#include "../TCPConnection.h"
#include "TCPState.h"
// #include "TCPStateMachine.h"
#include "TCPStateTimeWait.h"
class TCPStateClosing : public TCPState {
   public:
    TCPStateClosing(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "CLOSING";
        L_DEBUG("State created: " + NAME);
    }
    ~TCPStateClosing(){};

    bool onEvent(TCPEvent);
};
#endif