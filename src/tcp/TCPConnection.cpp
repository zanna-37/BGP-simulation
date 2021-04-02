#include "TCPConnection.h"

// #include "../entities/Device.h"

TCPConnection ::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine =
        new TCPStateMachine<TCPConnection, TCPState, TCPEvent>(this);
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() { delete stateMachine; }

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}

TCPState* TCPConnection::getCurrentState() {
    return stateMachine->getCurrentState();
}