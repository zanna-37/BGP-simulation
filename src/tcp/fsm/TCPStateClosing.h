#ifndef TCPSTATECLOSING_H
#define TCPSTATECLOSING_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateClosing : public TCPState {
   public:
    TCPStateClosing(TCPStateMachine* stateMachine);
    ~TCPStateClosing(){};

    bool onEvent(TCPEvent);
};
#endif