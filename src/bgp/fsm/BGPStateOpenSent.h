#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEOPENSENT_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEOPENSENT_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateOpenSent : public BGPState {
   private:
   public:
    BGPStateOpenSent(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        name = "OPEN_SENT";
        // L_DEBUG_CONN(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, stateMachine->connection->toString(), "State
        // created: " + name);
    };

    ~BGPStateOpenSent() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEOPENSENT_H
