#ifndef BGPSIMULATION_BGP_FSM_BGPSTATEIDLE_H
#define BGPSIMULATION_BGP_FSM_BGPSTATEIDLE_H

#include <string>

#include "../BGPEvent.h"
#include "BGPState.h"
#include "BGPStateMachine.h"


class BGPStateIdle : public BGPState {
   private:
   public:
    BGPStateIdle(BGPStateMachine* stateMachine) : BGPState(stateMachine) {
        name = "IDLE";
        // L_DEBUG(stateMachine->connection->owner->ID + " " +
        // stateMachine->name, "State created: " + name);
    };


    ~BGPStateIdle() override = default;

    bool onEvent(BGPEvent) override;
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATEIDLE_H
