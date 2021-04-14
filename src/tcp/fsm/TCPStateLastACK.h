#ifndef TCPSTATELASTACK_H
#define TCPSTATELASTACK_H


#include "TCPState.h"
#include "TCPStateMachine.h"
class TCPStateLastACK : public TCPState {
   public:
    TCPStateLastACK(TCPStateMachine* stateMachine);

    ~TCPStateLastACK(){};

    bool onEvent(TCPEvent);
};
#endif