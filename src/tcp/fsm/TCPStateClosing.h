#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATECLOSING_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATECLOSING_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateClosing : public TCPState {
   public:
    TCPStateClosing(TCPStateMachine* stateMachine);

    ~TCPStateClosing() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATECLOSING_H
