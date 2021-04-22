#include "BGPState.h"

void BGPState::dropTCPConnection() {
    stateMachine->connection->tcpConnection->enqueueEvent(TCPEvent::SendRST);
    stateMachine->connection->tcpConnection = nullptr;
}

void BGPState::initiateTCPConnection() {
    TCPConnection *tcpConnection = stateMachine->connection->owner->connect(
        stateMachine->connection->dstAddr, 179);
    stateMachine->connection->tcpConnection = tcpConnection;
}