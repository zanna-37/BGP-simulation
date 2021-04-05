#ifndef TCPSTATELISTEN_H
#define TCPSTATELISTEN_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateListen : public TCPState {
   public:
    TCPStateListen(TCPStateMachine* stateMachine);
    ~TCPStateListen(){};

    bool onEvent(TCPEvent);
};
#endif