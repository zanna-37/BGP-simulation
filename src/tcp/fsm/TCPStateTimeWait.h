#ifndef TCPSTATETIMEWAIT_H
#define TCPSTATETIMEWAIT_H


#include "TCPState.h"
#include "TCPStateMachine.h"

class TCPStateTimeWait : public TCPState {
   public:
    TCPStateTimeWait(TCPStateMachine* stateMachine);

    ~TCPStateTimeWait(){};

    bool onEvent(TCPEvent);
};
#endif