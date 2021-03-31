#ifndef TCPSTATECLOSED_H
#define TCPSTATECLOSED_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateListen.h"
#include "TCPStateMachine.h"
#include "TCPStateSYNSent.h"
class TCPStateClosed : public TCPState {
   public:
    TCPStateClosed(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "CLOSED";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateClosed(){};

    bool onEvent(TCPEvent);
};
#endif