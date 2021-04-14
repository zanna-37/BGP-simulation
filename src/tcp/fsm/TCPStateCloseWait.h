#ifndef TCPSTATECLOSEWAIT_H
#define TCPSTATECLOSEWAIT_H

#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateCloseWait : public TCPState {
   public:
    TCPStateCloseWait(TCPStateMachine* stateMachine);

    ~TCPStateCloseWait(){};

    bool onEvent(TCPEvent);
};
#endif