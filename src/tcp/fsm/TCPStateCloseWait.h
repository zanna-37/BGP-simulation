#ifndef TCPSTATECLOSEWAIT_H
#define TCPSTATECLOSEWAIT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateLastACK.h"
// #include "TCPStateMachine.h"
class TCPStateCloseWait : public TCPState {
   public:
    TCPStateCloseWait(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "CLOSE_WAIT";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateCloseWait(){};

    bool onEvent(TCPEvent);
};
#endif