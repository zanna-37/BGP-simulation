#ifndef TCPTIMER_H
#define TCPTIMER_H

#include "../utils/Timer.h"
#include "TCPEvent.h"
#include "fsm/TCPStateMachine.h"

// forward declarations
#include "fsm/TCPStateMachine.fwd.h"

class TCPTimer : public Timer<TCPStateMachine, TCPEvent> {
   public:
    TCPTimer(std::string          name,
             TCPStateMachine*     stateMachine,
             TCPEvent             eventToSendUponExpire,
             std::chrono::seconds totalDuration);
};
#endif