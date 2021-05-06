#ifndef BGPCONNECTION_H
#define BGPCONNECTION_H


#include <cassert>
#include <mutex>
#include <thread>

#include "../entities/Router.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "BGPEvent.h"
#include "fsm/BGPStateMachine.h"

// forward declarations
#include "../entities/Router.fwd.h"
#include "../socket/Socket.fwd.h"
#include "../tcp/TCPConnection.fwd.h"
#include "fsm/BGPStateMachine.fwd.h"

class BGPConnection {
   private:
    BGPStateMachine* stateMachine = nullptr;
    // other BGPConnection variables

   public:
    Router*           owner   = nullptr;
    std::string       name    = "BGPconnection";
    bool              running = false;
    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;

    std::chrono::seconds holdTime = 0s;

    Socket* connectedSocket = nullptr;

    std::thread* receivingThread = nullptr;

    // Constructors
    BGPConnection(Router* owner);

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

    void connect();

    void closeConnection();

    void receiveData();
    void sendData(std::stack<pcpp::Layer*>* layers);
};

#endif