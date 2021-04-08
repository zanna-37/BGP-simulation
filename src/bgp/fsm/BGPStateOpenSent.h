#ifndef BGPSTATEOPENSENT_H
#define BGPSTATEOPENSENT_H

#include "../../logger/Logger.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateOpenSent : public BGPState {
   private:
   public:
    // Constructor
    BGPStateOpenSent(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "OPEN_SENT";
        L_DEBUG(stateMachine->connection->owner->ID, "State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateOpenSent();

    bool onEvent(BGPEvent event);
};

#endif