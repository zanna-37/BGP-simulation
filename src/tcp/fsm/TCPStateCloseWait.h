#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATECLOSEWAIT_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATECLOSEWAIT_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateCloseWait : public TCPState {
   public:
    TCPStateCloseWait(TCPStateMachine* stateMachine);

    ~TCPStateCloseWait() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATECLOSEWAIT_H
