#include "TCPConnection.h"

TCPConnection ::TCPConnection() { stateMachine = new TCPStateMachine(this); }
TCPConnection::~TCPConnection() { delete stateMachine; }

void TCPConnection::enqueueEvent(TCPEvent event) {
    stateMachine->enqueueEvent(event);
}