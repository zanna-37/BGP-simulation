#ifndef BGPCONNECTION_H
#define BGPCONNECTION_H


#include <cassert>

#include "Event.h"
#include "fsm/BGPStateIdle.h"
#include "fsm/BGPStateMachine.h"

class BGPStateMachine;  // forward declaration

class BGPConnection {
   public:
   private:
    BGPStateMachine* stateMachine;
    // other BGPConnection variables

   public:
    // Constructors
    BGPConnection();

    // Destructor
    ~BGPConnection();

    /**
     * Enqueue an event handled by the BGPConnection State Machine.
     * @param event the event triggered
     */
    void enqueueEvent(Event event);
};

#endif