#ifndef BGPSTATEACTIVE_H
#define BGPSTATEACTIVE_H

#include "../BGPConnection.h"
#include "BGPState.h"
// #include "BGPStateMachine.h"

class BGPStateActive : public BGPState {
   private:
   public:
    // Constructor
    BGPStateActive(StateMachine<BGPConnection, BGPState, Event>* stateMachine)
        : BGPState(stateMachine) {
        NAME = "ACTIVE";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateActive();

    bool onEvent(Event event);
};

#endif