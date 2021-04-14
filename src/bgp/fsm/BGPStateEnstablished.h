#ifndef BGPSTATEENSTABLISHED_H
#define BGPSTATEENSTABLISHED_H

#include "../../logger/Logger.h"
#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateEnstablished : public BGPState {
   private:
   public:
    // Constructor
    BGPStateEnstablished(BGPStateMachine* stateMachine)
        : BGPState(stateMachine) {
        NAME = "ENSTABLISHED";
        L_DEBUG(stateMachine->connection->owner->ID, "State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateEnstablished();

    bool onEvent(BGPEvent event);
};

#endif