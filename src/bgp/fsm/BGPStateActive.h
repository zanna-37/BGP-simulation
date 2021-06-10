#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEACTIVE_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEACTIVE_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateActive : public BGPState {
   private:
   public:
    BGPStateActive(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        name = "ACTIVE";
        // L_DEBUG(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, "State created: " + name);
    };

    ~BGPStateActive() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEACTIVE_H
