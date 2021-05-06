#ifndef BGPAPPLICATION_H
#define BGPAPPLICATION_H

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "../entities/Router.fwd.h"
#include "../socket/Socket.h"
#include "BGPConnection.h"

// forward declarations
#include "BGPConnection.fwd.h"


class BGPApplication {
   public:
    std::vector<BGPConnection*> bgpConnections;

    std::vector<std::thread*> listeningThread;
    bool                      running = false;

    // BGP Routing Table

    std::string name = "BGPApplication";

    Router* router = nullptr;

    std::thread* applicationThread;

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