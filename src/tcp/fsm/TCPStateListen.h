#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATELISTEN_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATELISTEN_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateListen : public TCPState {
   public:
    TCPStateListen(TCPStateMachine* stateMachine);

    ~TCPStateListen() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATELISTEN_H
