#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../../logger/Logger.h"
#include "../Event.h"
#include "BGPStateMachine.h"

class BGPStateMachine;  // forward declaration

class BGPState {
   public:
    BGPStateMachine* stateMachine;
    string           NAME;
    BGPState(BGPStateMachine* stateMachine) : stateMachine(stateMachine){};
    virtual ~BGPState() {}

    /**
     * The current state handle the event received and returns if everything went fine (the error was handled correctly). Called by the BGPStateMachine event hendler.
     * @param event the event triggered.
     * @return boolean indicating if the event was handled correctly
     */
    virtual bool onEvent(Event) = 0;

};

#endif