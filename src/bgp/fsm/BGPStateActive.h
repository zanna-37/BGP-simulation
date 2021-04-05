#ifndef BGPSTATEACTIVE_H
#define BGPSTATEACTIVE_H

#include "../BGPConnection.h"
#include "BGPState.h"
// #include "BGPStateMachine.h"

class BGPStateActive : public BGPState {
   private:
   public:
    // Constructor
    BGPStateActive(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "ACTIVE";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateActive();

    bool onEvent(BGPEvent event);
};

#endif