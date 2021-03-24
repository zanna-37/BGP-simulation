#include "BGPConnection.h"


BGPConnection::BGPConnection() { stateMachine = new BGPStateMachine(this); }
BGPConnection::~BGPConnection() { delete stateMachine; }

void BGPConnection::enqueueEvent(Event event) {
    stateMachine->enqueueEvent(event);
}
