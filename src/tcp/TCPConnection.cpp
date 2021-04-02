#include "TCPConnection.h"

// #include "../entities/Device.h"

TCPConnection ::TCPConnection(Device* owner) : owner(owner) {
    stateMachine = new StateMachine<TCPConnection, TCPState, TCPEvent>(this);
    stateMachine->changeState(new TCPStateClosed(stateMachine));
}

TCPConnection::~TCPConnection() { delete stateMachine; }

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}

TCPState* TCPConnection::getCurrentState() {
    return stateMachine->getCurrentState();
}