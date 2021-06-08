#ifndef BGP_SIMULATION_TCP_FSM_TCPSTATE_H
#define BGP_SIMULATION_TCP_FSM_TCPSTATE_H

#include <stack>
#include <string>

#include "../TCPEvent.h"

// forward declarations
#include "TCPStateMachine.fwd.h"


class TCPState {
   public:
    /**
     * The TCPStateMachine owner of the state
     */
    TCPStateMachine* stateMachine;
    /**
     * The string name of the state:
     * - CLOSED
     * - CLOSE-WAIT
     * - CLOSING
     * - ENSTABLISHED
     * - FIN-WAIT-1
     * - FIN-WAIT-2
     * - LAST-ACK
     * - LISTEN
     * - SYN-RECEIVED
     * - SYN-SENT
     * - TIME-WAIT
     */
    std::string name;

    TCPState(TCPStateMachine* stateMachine) : stateMachine(stateMachine){};

    virtual ~TCPState() = default;

    /**
     * Method called by each state to handle a specific TCP event triggered in
     * the state machine
     * @warning it should be called only inside the state machine
     * @param event the event to be handled
     * @return \a true if the event was handled correctly, \a false otherwise
     */
    virtual bool onEvent(TCPEvent) = 0;
};

#endif  // BGP_SIMULATION_TCP_FSM_TCPSTATE_H
