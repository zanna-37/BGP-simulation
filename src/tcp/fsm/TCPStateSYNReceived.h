#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATESYNRECEIVED_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATESYNRECEIVED_H

#include "../TCPEvent.h"
#include "TCPState.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPStateSYNReceived : public TCPState {
   public:
    TCPStateSYNReceived(TCPStateMachine* stateMachine);

    ~TCPStateSYNReceived() override = default;

    bool onEvent(TCPEvent) override;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATESYNRECEIVED_H
