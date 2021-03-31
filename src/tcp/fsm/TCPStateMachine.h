#ifndef TCPSTATEMACHINE_H
#define TCPSTATEMACHINE_H

#include <cassert>
#include <condition_variable>
#include <queue>
#include <thread>

#include "../../logger/Logger.h"
#include "../TCPConnection.h"
#include "../TCPEvent.h"

class TCPState;       // forward declaration
class TCPConnection;  // forward declaration
class TCPStateMachine {
   private:
    TCPConnection* connection;

    TCPState* currentState = nullptr;

    std::thread*            eventHandler = nullptr;
    std::mutex              eventQueue_mutex;
    std::condition_variable eventQueue_ready;
    queue<TCPEvent>         eventQueue;


   public:
    TCPStateMachine(TCPConnection* connection);

    ~TCPStateMachine();

    void changeState(TCPState* newState);

    void enqueueEvent(TCPEvent);
};
#endif