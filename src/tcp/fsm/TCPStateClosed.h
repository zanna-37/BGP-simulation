#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATECLOSED_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATECLOSED_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateClosed : public TCPState {
   public:
    TCPStateClosed(TCPStateMachine* stateMachine);

    ~TCPStateClosed() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATECLOSED_H
