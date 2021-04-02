#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../../logger/Logger.h"
#include "../Event.h"
// #include "BGPStateMachine.h"
#include "../../fsm/StateMachine.h"
template <class Connection, class State, class Event>
class StateMachine;  // forward declaration

class BGPState {
   public:
    StateMachine<BGPConnection, BGPState, Event>* stateMachine;
    string                                        NAME;
    BGPState(StateMachine<BGPConnection, BGPState, Event>* stateMachine)
        : stateMachine(stateMachine){};
    virtual ~BGPState() {}

    /**
     * The current state handles the event received and returns if everything
     * went fine (the error was handled correctly).
     * @warning this method should only be called by the BGPStateMachine event
     * handler.
     * @param event the event triggered.
     * @return boolean indicating if the event was handled correctly
     */
    virtual bool onEvent(Event) = 0;
};

#endif