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
     * The current state handles the event received and returns if everything went fine (the error was handled correctly). 
     * @warning this method should only be called by the BGPStateMachine event handler.
     * @param event the event triggered.
     * @return boolean indicating if the event was handled correctly
     */
    virtual bool onEvent(Event) = 0;

};

#endif