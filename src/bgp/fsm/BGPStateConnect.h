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
        NAME = "CONNECT";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateConnect();

    bool onEvent(BGPEvent event);
};

#endif