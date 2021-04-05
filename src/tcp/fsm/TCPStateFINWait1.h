#ifndef TCPSTATEFINWAIT1_H
#define TCPSTATEFINWAIT1_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateFINWait1 : public TCPState {
   public:
    TCPStateFINWait1(TCPStateMachine* stateMachine);

    ~TCPStateFINWait1(){};

    bool onEvent(TCPEvent);
};
#endif