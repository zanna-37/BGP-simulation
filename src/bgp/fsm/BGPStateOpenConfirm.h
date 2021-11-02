#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEOPENCONFIRM_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEOPENCONFIRM_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateOpenConfirm : public BGPState {
   private:
   public:
    BGPStateOpenConfirm(BGPStateMachine* stateMachine)
        : BGPState(stateMachine) {
        name = "OPEN_CONFIRM";
        // L_DEBUG_CONN(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, stateMachine->connection->toString(), "State
        // created: " + name);
    };


    ~BGPStateOpenConfirm() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEOPENCONFIRM_H
