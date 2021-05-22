#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../BGPEvent.h"
#include "BGPStateMachine.h"

// forward declarations
#include "BGPStateMachine.fwd.h"

class BGPState {
   public:
    BGPStateMachine* stateMachine;
    std::string      name;
    BGPState(BGPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~BGPState() {}

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

#endif