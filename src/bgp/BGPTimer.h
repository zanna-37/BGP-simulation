#ifndef BGPSIMULATION_BGP_BGPTIMER_H
#define BGPSIMULATION_BGP_BGPTIMER_H

#include <chrono>
#include <string>

#include "../utils/Timer.h"
#include "BGPEvent.h"

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

#endif  // BGPSIMULATION_BGP_BGPTIMER_H
