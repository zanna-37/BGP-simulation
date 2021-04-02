#ifndef TCPSTATECLOSED_H
#define TCPSTATECLOSED_H

#include "../TCPConnection.h"
#include "TCPState.h"
#include "TCPStateListen.h"
// #include "TCPStateMachine.h"
#include "TCPStateSYNSent.h"
class TCPStateClosed : public TCPState {
   public:
    TCPStateClosed(
        StateMachine<TCPConnection, TCPState, TCPEvent>* stateMachine);

    ~TCPStateClosed(){};

    bool onEvent(TCPEvent);
};
#endif