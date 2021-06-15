#ifndef BGPSIMULATION_BGP_BGPCONNECTION_H
#define BGPSIMULATION_BGP_BGPCONNECTION_H

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <thread>

#include "BGPEvent.h"
#include "IpAddress.h"
#include "Layer.h"

// forward declarations
#include "../entities/Router.fwd.h"
#include "../socket/Socket.fwd.h"
#include "./BGPApplication.fwd.h"
#include "fsm/BGPStateMachine.fwd.h"

using namespace std::chrono_literals;


class BGPConnection {
   private:
    BGPStateMachine* stateMachine = nullptr;
    // other BGPConnection variables

    BGPApplication* bgpApplication;

    std::thread* listeningThread;

    /**
     * The thread that manages any incoming application packet.
     */
    std::thread* receivingThread = nullptr;

    std::thread* connectThread = nullptr;

   public:
    /**
     * The owner of the connection, necessary in simulation context
     */
    Router*           owner   = nullptr;
    std::string       name    = "BGPconnection";
    std::atomic<bool> running = {false};
    pcpp::IPv4Address srcAddr = pcpp::IPv4Address::Zero;  // TODO remove(?)
    pcpp::IPv4Address dstAddr = pcpp::IPv4Address::Zero;  // TODO remove(?)
    uint16_t          srcPort = 179;                      // TODO remove!!

    std::chrono::seconds holdTime = 0s;

    std::mutex connectedSocket_mutex;

    /**
     * The newly created connected socket is assigned to this attribute, when
     * the TCP connection is established
     */
    Socket* connectedSocket = nullptr /*GUARDED_BY(connectedSocket_mutex)*/;

    // Constructors
    BGPConnection(Router* owner, BGPApplication* bgpApplication);

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
    void asyncConnectToPeer();

    /**
     * Close the BGP connection and notifies the state machine
     */
    void closeConnection();

    /**
     * Starts the receiving thread. This thread will wait for new messages
     * incoming
     */
    void startReceivingThread();

    /**
     * Send BGP application data to the TCP layer through the socket
     * @param layers the vector abstraction of the internet packet that is built
     * from the application layer.
     */
    void sendData(
        std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers);
    void listenForRemotelyInitiatedConnections();
    void dropConnection();
    void shutdown();
};

#endif  // BGPSIMULATION_BGP_BGPCONNECTION_H
