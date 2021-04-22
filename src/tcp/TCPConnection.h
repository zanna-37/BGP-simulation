#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H

#include <IPv4Layer.h>
#include <TcpLayer.h>

#include <stack>

#include "../entities/Device.h"
#include "TCPEvent.h"
#include "fsm/TCPState.h"
#include "fsm/TCPStateMachine.h"

// forward declarations
#include "../entities/Device.fwd.h"
#include "fsm/TCPState.fwd.h"
#include "fsm/TCPStateMachine.fwd.h"

class TCPConnection {
   public:
    /**
     * The connection state machine. It is based on RFC 793
     */
    TCPStateMachine* stateMachine = nullptr;

    /**
     * The owner of the connection
     */
    Device* owner = nullptr;
    /**
     * The connection source IP address
     */
    pcpp::IPv4Address* srcAddr = nullptr;
    /**
     * The connection source port
     */
    uint16_t srcPort = 0;
    /**
     * The connection destination IP address
     */
    pcpp::IPv4Address* dstAddr = nullptr;
    /**
     * The connection source port
     */
    uint16_t dstPort = 0;
    // TODO remove, we do not need it
    uint16_t BGPPort = 179;

    TCPConnection(Device* owner);
    ~TCPConnection();
    /**
     * Enqueue a TCPEvent in the TCPStateMachine event queue
     * @param event the event to trigger
     */
    void enqueueEvent(TCPEvent);
    /**
     * Take the TCP header as input and return a \a unint8_t indicating what
     * flags are set and what are not
     * @param tcpHeader the TCP header
     * @return a \a uint8_t value indicating what flags are set in the header
     */
    uint8_t parseTCPFlags(pcpp::tcphdr* tcpHeader);

    /**
     * Process a newly arrived packet at the TCP layer, based on the flags int
     * the TCP header. It enqueues event accordingly, based on TCPStateMachine
     * defined in RFC 793
     */
    void processMessage(std::stack<pcpp::Layer*>* layers);

    /**
     * Return a pointer to the current state of the connection
     * @return pointer to the current state of the connection
     */
    TCPState* getCurrentState();


    void sendPacket(std::stack<pcpp::Layer*>* layers);
};

#endif