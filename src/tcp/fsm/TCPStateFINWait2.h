#ifndef TCPSTATEFINWAIT2_H
#define TCPSTATEFINWAIT2_H

#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateFINWait2 : public TCPState {
   public:
    TCPStateFINWait2(TCPStateMachine* stateMachine);

    ~TCPStateFINWait2(){};

    bool onEvent(TCPEvent);
};
#endif