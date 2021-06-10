#ifndef BGPSIMULATION_BGP_FSM_BGPSTATE_H
#define BGPSIMULATION_BGP_FSM_BGPSTATE_H

#include <string>

#include "../BGPEvent.h"

// forward declarations
#include "BGPStateMachine.fwd.h"


class BGPState {
   public:
    BGPStateMachine* stateMachine;
    std::string      name;
    BGPState(BGPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~BGPState() = default;

    /**
     * The current state handles the event received and returns if everything
     * went fine (the error was handled correctly).
     * @warning this method should only be called by the BGPStateMachine event
     * handler.
     * @param event the event triggered.
     * @return boolean indicating if the event was handled correctly
     */
    virtual bool onEvent(BGPEvent) = 0;

    // void initiateTCPConnection();

    // void dropTCPConnection();
};

#endif  // BGPSIMULATION_BGP_FSM_BGPSTATE_H
