#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEESTABLISHED_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEESTABLISHED_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateEstablished : public BGPState {
   private:
   public:
    BGPStateEstablished(BGPStateMachine* stateMachine)
        : BGPState(stateMachine) {
        name = "ESTABLISHED";
        // L_DEBUG_CONN(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, stateMachine->connection->toString(), "State
        // created: " + name);
    };


    ~BGPStateEstablished() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEESTABLISHED_H
