#ifndef TCPSTATECLOSED_H
#define TCPSTATECLOSED_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateClosed : public TCPState {
   private:
   public:
    TCPStateClosed(TCPStateMachine* stateMachine) : TCPState(stateMachine) {
        NAME = "CLOSED";
        L_DEBUG("State created: " + NAME);
    }

    ~TCPStateClosed(){};

    bool onEvent(TCPEvent);
};
#endif