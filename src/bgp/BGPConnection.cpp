#include "BGPConnection.h"

#include "fsm/BGPStateIdle.h"
BGPConnection::BGPConnection(Device* owner) : owner(owner) {
    stateMachine = new BGPStateMachine(this);
    this->stateMachine->changeState(new BGPStateIdle(this->stateMachine));
    stateMachine->start();
}
BGPConnection::~BGPConnection() { delete stateMachine; }

void BGPConnection::enqueueEvent(BGPEvent event) {
    stateMachine->enqueueEvent(event);
}
