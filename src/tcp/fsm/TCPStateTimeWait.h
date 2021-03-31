#ifndef TCPSTATETIMEWAIT_H
#define TCPSTATETIMEWAIT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateTimeWait : public TCPState {
   private:
   public:
    TCPStateTimeWait(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "TIME-WAIT";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateTimeWait(){};

    bool onEvent(TCPEvent);
};
#endif