#include "TCPConnection.h"

#include "fsm/TCPStateClosed.h"

TCPConnection ::TCPConnection(Device* owner) : owner(owner) {
    this->stateMachine = new TCPStateMachine(this);
    this->stateMachine->changeState(new TCPStateClosed(this->stateMachine));
    this->stateMachine->start();
}

TCPConnection::~TCPConnection() { delete stateMachine; }

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}

TCPState* TCPConnection::getCurrentState() {
    return stateMachine->getCurrentState();
}