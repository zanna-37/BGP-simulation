#ifndef TCPSTATECLOSEWAIT_H
#define TCPSTATECLOSEWAIT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateCloseWait : public TCPState {
   private:
   public:
    TCPStateCloseWait(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "CLOSE-WAIT";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateCloseWait(){};

    bool onEvent(TCPEvent);
};
#endif