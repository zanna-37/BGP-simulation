#ifndef TCPSTATE_H
#define TCPSTATE_H


#include <IPLayer.h>
#include <TcpLayer.h>

#include <stack>

#include "../TCPEvent.h"
#include "TCPStateMachine.h"

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
    virtual ~TCPState() {}

    /**
     * Method called by each state to handle a specific TCP event triggered in
     * the state machine
     * @warning it should be called only inside the state machine
     * @param event the event to be handled
     * @return \a true if the event was handled correctly, \a false otherwise
     */
    virtual bool onEvent(TCPEvent) = 0;

    //  pcpp::TcpLayer* craftTCPLayer(uint16_t srcPort,
    //                                uint16_t dstPort,
    //                                int      flags);
};
#endif
