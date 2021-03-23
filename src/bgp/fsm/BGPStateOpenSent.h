#ifndef BGPSTATEOPENSENT_H
#define BGPSTATEOPENSENT_H

#include "../BGPConnection.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateOpenSent : public BGPState {
   private:
   public:
    // Constructor
    BGPStateOpenSent(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        NAME = "OPEN_SENT";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateOpenSent();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event event);
};

#endif