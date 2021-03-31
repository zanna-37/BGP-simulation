#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include "TCPEvent.h"
#include "fsm/TCPStateClosed.h"
#include "fsm/TCPStateMachine.h"

class TCPStateMachine;  // forward declaration
class TCPConnection {
   public:
    TCPStateMachine* stateMachine;

    TCPConnection();
    ~TCPConnection();

    void enqueueEvent(TCPEvent);
};

#endif