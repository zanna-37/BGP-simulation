#ifndef BGPTIMER_H
#define BGPTIMER_H

#include "../utils/Timer.h"
#include "BGPEvent.h"
#include "fsm/BGPStateMachine.h"

// forward declarations
#include "fsm/BGPStateMachine.fwd.h"

class BGPTimer : public Timer<BGPStateMachine, BGPEvent> {
   public:
    BGPTimer(std::string          name,
             BGPStateMachine*     stateMachine,
             BGPEvent             eventToSendUponExpire,
             std::chrono::seconds totalDuration);
};

#endif