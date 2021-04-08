#ifndef BGPSTATEACTIVE_H
#define BGPSTATEACTIVE_H

#include "../../logger/Logger.h"
#include "BGPState.h"

class BGPStateActive : public BGPState {
   private:
   public:
    // Constructor
    BGPStateActive(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "ACTIVE";
        L_DEBUG(stateMachine->connection->owner->ID, "State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateActive();

    bool onEvent(BGPEvent event);
};

#endif