#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATESYNSENT_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATESYNSENT_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateSYNSent : public TCPState {
   public:
    TCPStateSYNSent(TCPStateMachine* stateMachine);

    ~TCPStateSYNSent() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATESYNSENT_H
