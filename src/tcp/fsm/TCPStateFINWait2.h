#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT2_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT2_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateFINWait2 : public TCPState {
   public:
    TCPStateFINWait2(TCPStateMachine* stateMachine);

    ~TCPStateFINWait2() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATEFINWAIT2_H
