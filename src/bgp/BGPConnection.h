#ifndef BGPCONNECTION_H
#define BGPCONNECTION_H


#include <cassert>

#include "../entities/Device.h"
#include "../tcp/TCPConnection.h"
#include "BGPEvent.h"
#include "fsm/BGPStateMachine.h"


// forward declarations
#include "../entities/Device.fwd.h"
#include "../tcp/TCPConnection.fwd.h"
#include "fsm/BGPStateMachine.fwd.h"

class BGPConnection {
   public:
   private:
    BGPStateMachine* stateMachine = nullptr;
    // other BGPConnection variables

   public:
    Device* owner;

    std::string          dstAddr;
    TCPConnection*       tcpConnection = nullptr;
    std::chrono::seconds holdTime      = 0s;
    // Constructors
    BGPConnection(Device* owner);

    // Destructor
    ~BGPConnection();

    /**
     * Enqueue an event handled by the BGPConnection State Machine.
     * @param event the event triggered
     */
    void enqueueEvent(BGPEvent event);

    /**
     * Handle BGP message
     * @param layers the BGP layers contained in the received message
     */
    void processMessage(std::stack<pcpp::Layer*>* layers);
};

#endif