#ifndef BGPSTATE_H
#define BGPSTATE_H

#include "../../logger/Logger.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "BGPStateMachine.h"
template <class Connection, class State, class Event>
class BGPStateMachine;  // forward declaration
class BGPConnection;    // forward declaration

class BGPState {
   public:
    BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine;
    string                                              NAME;
    BGPState(BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine)
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
    virtual bool onEvent(BGPEvent) = 0;
};

#endif