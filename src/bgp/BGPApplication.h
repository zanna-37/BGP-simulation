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
    /**
     * List of the bgp connections active when the application is running
     */
    std::vector<BGPConnection*> bgpConnections;

    /**
     * A listening thread is created for each new connection, but it
     */
    std::vector<std::thread*> listeningThreads;
    bool                      running = false;

    // BGP Routing Table

    std::string name = "BGPApplication";

    Router* router = nullptr;

    std::mutex runningMutex;


    BGPApplication(Router* router);
    ~BGPApplication();

    void passiveOpen();
    void activeOpen();

    void bindSocketToBGPConnection(Socket* socket);


    void stop();

    void collisionDetection(BGPConnection* connectionToCheck);
};

#endif