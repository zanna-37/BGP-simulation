#ifndef BGPSTATEIDLE_H
#define BGPSTATEIDLE_H

#include "../BGPConnection.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateIdle : public BGPState {
   private:
   public:
    // Constructor
    BGPStateIdle(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "IDLE";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateIdle();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event);
};

#endif