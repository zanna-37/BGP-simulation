#ifndef BGPSIMULATION_TCP_TCPTIMER_H
#define BGPSIMULATION_TCP_TCPTIMER_H

#include <chrono>
#include <string>

#include "../utils/Timer.h"
#include "TCPEvent.h"
#include "fsm/TCPStateMachine.h"


class TCPTimer : public Timer<TCPStateMachine, TCPEvent> {
   public:
    TCPTimer(std::string          name,
             TCPStateMachine*     stateMachine,
             TCPEvent             eventToSendUponExpire,
             std::chrono::seconds totalDuration);
};

#endif  // BGPSIMULATION_TCP_TCPTIMER_H
