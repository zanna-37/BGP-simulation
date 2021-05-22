#ifndef BGPAPPLICATION_H
#define BGPAPPLICATION_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "../entities/Router.h"
#include "../socket/Socket.h"
#include "BGPConnection.h"

// forward declarations
#include "../entities/Router.fwd.h"
#include "BGPConnection.fwd.h"


class BGPApplication {
   public:
    static const uint16_t BGPDefaultPort = 179;

    /**
     * List of the bgp connections active when the application is running
     */
    std::vector<BGPConnection*> bgpConnections;

    /**
     * A listening thread is created for each new connection, but it
     */
    std::vector<std::thread> listeningThreads;

    std::vector<Socket*> listeningSockets;

    bool running = false;

    // BGP Routing Table

    std::string name = "BGPApp";

    /**
     * The router that started the application
     */
    Router* router = nullptr;


    BGPApplication(Router* router);
    ~BGPApplication();

    /**
     * The BGP application start without immediately connecting to the peers,
     * but waiting for the peer to connect
     */
    void passiveOpen();

    /**
     * The BGP application starts by actively connecting to the remote peers
     * // TODO implement it
     */
    void activeOpen();

    /**
     * Find the BGPconnection related to the input socket and binds it to it. It
     * is used when the listening socket receive an incoming connection and
     * accept the connection. The resulting socket, crated by the accept method,
     * needs to be bound to the respective BGP conection
     * @param socket the connection to bind to the connection
     */
    void bindSocketToBGPConnection(Socket* socket);


    /**
     * // TODO to implement. Stops all the application and all the BGP
     * connections active
     */
    void stop();

    /**
     * Check if a BGP connection will cause a collision with another connection
     * already created and close the connection based on the decision method
     * provided in RFC 4271
     * @param connectionToCheck the BGP connection to be checked
     */
    void collisionDetection(BGPConnection* connectionToCheck);
};

#endif