#include "BGPConnection.h"


BGPConnection::BGPConnection(Device* owner) : owner(owner) {
    stateMachine = new BGPStateMachine<BGPConnection, BGPState, BGPEvent>(this);
    stateMachine->start();
}
BGPConnection::~BGPConnection() { delete stateMachine; }

void BGPConnection::enqueueEvent(BGPEvent event) {
    stateMachine->enqueueEvent(event);
}
