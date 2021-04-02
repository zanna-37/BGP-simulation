#ifndef BGPSTATEIDLE_H
#define BGPSTATEIDLE_H

#include "../BGPConnection.h"
#include "BGPState.h"
// #include "BGPStateMachine.h"

class BGPStateIdle : public BGPState {
   private:
   public:
    // Constructor
    BGPStateIdle(
        BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine)
        : BGPState(stateMachine) {
        NAME = "IDLE";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateIdle();

    bool onEvent(BGPEvent);
};

#endif