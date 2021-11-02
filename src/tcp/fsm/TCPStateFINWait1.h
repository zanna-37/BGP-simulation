#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT1_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT1_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateFINWait1 : public TCPState {
   public:
    TCPStateFINWait1(TCPStateMachine* stateMachine);

    ~TCPStateFINWait1() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT1_H
