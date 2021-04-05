#ifndef TCPSTATEENSTABLISHED_H
#define TCPSTATEENSTABLISHED_H

#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateEnstablished : public TCPState {
   public:
    TCPStateEnstablished(TCPStateMachine* stateMachine);
    ~TCPStateEnstablished(){};

    bool onEvent(TCPEvent);
};
#endif