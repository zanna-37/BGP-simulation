#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATEESTABLISHED_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATEESTABLISHED_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateEstablished : public TCPState {
   public:
    TCPStateEstablished(TCPStateMachine* stateMachine);

    ~TCPStateEstablished() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATEESTABLISHED_H
