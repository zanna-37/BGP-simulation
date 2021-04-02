#ifndef TCPSTATETIMEWAIT_H
#define TCPSTATETIMEWAIT_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateClosed.h"
// #include "TCPStateMachine.h"
class TCPStateTimeWait : public TCPState {
   public:
    TCPStateTimeWait(
        StateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine)
        : TCPState(stateMachine) {
        NAME = "TIME-WAIT";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateTimeWait(){};

    bool onEvent(TCPEvent);
};
#endif