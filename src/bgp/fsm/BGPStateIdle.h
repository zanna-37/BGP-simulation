#ifndef BGPSTATEIDLE_H
#define BGPSTATEIDLE_H

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateIdle : public BGPState {
   private:
   public:
    // Constructor
    BGPStateIdle(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "IDLE";
        L_DEBUG(stateMachine->connection->owner->ID, "State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateIdle();

    bool onEvent(BGPEvent);
};

#endif