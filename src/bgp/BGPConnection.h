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
    /**
     * The owner of the connection, necessary in simulation context
     */
    Router*           owner   = nullptr;
    std::string       name    = "BGPconnection";
    bool              running = false;
    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;

    std::chrono::seconds holdTime = 0s;

    /**
     * The newly created connected socket is assigned to this attribute, when
     * the TCP connection is enstablished
     */
    Socket* connectedSocket = nullptr;

    /**
     * The thread that manages any incoming application packet.
     */
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
    void processMessage(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);

    /**
     * Client side connection of TCP, used by the router that wants to initiate
     * the connection
     */
    void connect();

    /**
     * Close the BGP connection and notifies the state machine
     */
    void closeConnection();

    /**
     * Starts the receiving thread. This thread will wait for new messages
     * incoming
     */
    void receiveData();

    /**
     * Send BGP application data to the TCP layer through the socket
     * @param layers the vector abstraction of the internet packet that is built
     * from the application layer.
     */
    void sendData(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);
};

#endif