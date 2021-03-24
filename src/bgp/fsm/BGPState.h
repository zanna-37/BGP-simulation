#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../../logger/Logger.h"
#include "../Event.h"
#include "BGPStateMachine.h"

class BGPStateMachine;  // forward declaration

class BGPState {
   public:
    BGPStateMachine* stateMachine;
    string           NAME;
    BGPState(BGPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~BGPState() {}

    virtual bool onEvent(Event) = 0;

    // You can implement here some global reactions to Events
};

#endif