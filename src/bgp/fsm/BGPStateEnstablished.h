#ifndef BGPSTATEENSTABLISHED_H
#define BGPSTATEENSTABLISHED_H

#include "../BGPConnection.h"
#include "BGPState.h"
// #include "BGPStateMachine.h"

class BGPStateEnstablished : public BGPState {
   private:
   public:
    // Constructor
    BGPStateEnstablished(
        StateMachine<BGPConnection, BGPState, Event>* stateMachine)
        : BGPState(stateMachine) {
        NAME = "ENSTABLISHED";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateEnstablished();

    bool onEvent(Event event);
};

#endif