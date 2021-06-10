#ifndef BGPSIMULATION_BGP_FSM_BGPSTATECONNECT_H
#define BGPSIMULATION_BGP_FSM_BGPSTATECONNECT_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateConnect : public BGPState {
   private:
   public:
    BGPStateConnect(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        name = "CONNECT";
        // L_DEBUG(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, "State created: " + name);
    };

    ~BGPStateConnect() override = default;

    bool onEvent(BGPEvent event) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATECONNECT_H
