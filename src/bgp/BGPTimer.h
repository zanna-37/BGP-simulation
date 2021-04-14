#ifndef BGPTIMER_H
#define BGPTIMER_H

#include "../utils/Timer.h"
#include "BGPEvent.h"
#include "fsm/BGPStateMachine.h"

// forward declarations
#include "fsm/BGPStateMachine.fwd.h"
/**
 * Implementation of template Timer for the timers used by the BGP State Machine
 */
class BGPTimer : public Timer<BGPStateMachine, BGPEvent> {
   public:
    BGPTimer(std::string          name,
             BGPStateMachine*     stateMachine,
             BGPEvent             eventToSendUponExpire,
             std::chrono::seconds totalDuration);
};

#endif