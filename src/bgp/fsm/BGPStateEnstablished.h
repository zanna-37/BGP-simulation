#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEENSTABLISHED_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEENSTABLISHED_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateEnstablished : public BGPState {
   private:
   public:
    BGPStateEnstablished(BGPStateMachine* stateMachine)
        : BGPState(stateMachine) {
        name = "ENSTABLISHED";
        // L_DEBUG(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, "State created: " + name);
    };


    ~BGPStateEnstablished() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEENSTABLISHED_H
