#ifndef TCPSTATECLOSED_H
#define TCPSTATECLOSED_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateClosed : public TCPState {
   public:
    TCPStateClosed(TCPStateMachine* stateMachine);

    ~TCPStateClosed(){};

    bool onEvent(TCPEvent);
};
#endif