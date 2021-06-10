#ifndef BGPSIMULATION_TCP_FSM_TCPSTATETIMEWAIT_H
#define BGPSIMULATION_TCP_FSM_TCPSTATETIMEWAIT_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateTimeWait : public TCPState {
   public:
    TCPStateTimeWait(TCPStateMachine* stateMachine);

    ~TCPStateTimeWait() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGPSIMULATION_TCP_FSM_TCPSTATETIMEWAIT_H
