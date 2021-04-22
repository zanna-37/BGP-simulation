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
        name = "ENSTABLISHED";
        L_DEBUG(stateMachine->connection->owner->ID + " " + stateMachine->name,
                "State created: " + name);
    };

    // Deconstructor
    ~BGPStateEnstablished();

    bool onEvent(BGPEvent event);
};

#endif