#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../BGPEvent.h"
#include "BGPStateMachine.fwd.h"
#include "BGPStateMachine.h"

class BGPState {
   public:
    BGPStateMachine* stateMachine;
    string           NAME;
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
};

#endif