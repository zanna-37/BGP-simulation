#ifndef TCPSTATESYNRECEIVED_H
#define TCPSTATESYNRECEIVED_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateSYNReceived : public TCPState {
   public:
    TCPStateSYNReceived(TCPStateMachine* stateMachine);
    ~TCPStateSYNReceived(){};

    bool onEvent(TCPEvent);
};
#endif