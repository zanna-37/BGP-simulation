#ifndef BGPSTATEOPENCONFIRM_H
#define BGPSTATEOPENCONFIRM_H


#include "../BGPConnection.h"
#include "BGPState.h"
#include "BGPStateMachine.h"

class BGPStateOpenConfirm : public BGPState {
   private:
   public:
    // Constructor
    BGPStateOpenConfirm(BGPStateMachine* stateMachine)
        : BGPState(stateMachine) {
        NAME = "OPEN_CONFIRM";
        L_DEBUG("State created: " + NAME);
    };

    // Deconstructor
    ~BGPStateOpenConfirm();

    void enter();
    void execute();
    void exit();

    bool onEvent(Event event);
};

#endif