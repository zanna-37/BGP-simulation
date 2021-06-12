#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATELASTACK_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATELASTACK_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateLastACK : public TCPState {
   public:
    TCPStateLastACK(TCPStateMachine* stateMachine);

    ~TCPStateLastACK() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATELASTACK_H
