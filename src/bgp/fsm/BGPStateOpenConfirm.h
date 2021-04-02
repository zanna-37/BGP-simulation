#ifndef BGPSTATEOPENCONFIRM_H
#define BGPSTATEOPENCONFIRM_H


#include "../BGPConnection.h"
#include "BGPState.h"
// #include "BGPStateMachine.h"

class BGPStateOpenConfirm : public BGPState {
   private:
   public:
    // Constructor
    BGPStateOpenConfirm(
        BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine)
        : BGPState(stateMachine) {
        NAME = "OPEN_CONFIRM";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateOpenConfirm();

    bool onEvent(BGPEvent event);
};

#endif