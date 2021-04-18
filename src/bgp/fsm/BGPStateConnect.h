#ifndef BGPSTATECONNECT_H
#define BGPSTATECONNECT_H

#include "../../logger/Logger.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateConnect : public BGPState {
   private:
   public:
    // Constructor
    BGPStateConnect(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        name = "CONNECT";
        L_DEBUG(stateMachine->connection->owner->ID, "State created: " + name);
    };

    // Deconstructor
    ~BGPStateConnect();

    bool onEvent(BGPEvent event);
};

#endif