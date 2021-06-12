#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATEENSTABLISHED_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATEENSTABLISHED_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateEnstablished : public TCPState {
   public:
    TCPStateEnstablished(TCPStateMachine* stateMachine);

    ~TCPStateEnstablished() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATEENSTABLISHED_H
