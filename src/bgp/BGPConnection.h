#ifndef BGPCONNECTION_H
#define BGPCONNECTION_H


#include <cassert>

#include "BGPEvent.h"
#include "fsm/BGPStateIdle.h"
#include "fsm/BGPStateMachine.h"

template <class Connection, class State, class Event>
class BGPStateMachine;  // forward declaration
class BGPState;
class Device;
class BGPConnection {
   public:
   private:
    BGPStateMachine<BGPConnection, BGPState, BGPEvent>* stateMachine = nullptr;
    // other BGPConnection variables

   public:
    Device* owner;
    // Constructors
    BGPConnection(Device* owner);

    // Destructor
    ~BGPConnection();

    /**
     * Enqueue an event handled by the BGPConnection State Machine.
     * @param event the event triggered
     */
    void enqueueEvent(BGPEvent event);
};

#endif