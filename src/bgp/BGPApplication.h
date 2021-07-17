#ifndef BGPSIMULATION_BGP_BGPAPPLICATION_H
#define BGPSIMULATION_BGP_BGPAPPLICATION_H

#include <atomic>
#include <cstdint>
#include <string>
#include <vector>

#include "../socket/Socket.h"
#include "IpAddress.h"

// forward declarations
#include "../entities/Router.fwd.h"
#include "BGPConnection.fwd.h"


class BGPApplication {
   private:
    // The value of the BGP Identifier is determined upon
    // startup and is the same for every local interface and BGP peer.
    // TODO define if we want to leave the first nework card as bgpIdentifier
    pcpp::IPv4Address BGPIdentifier = nullptr;

   public:
    static const int BGPDefaultPort = 179;  // TODO change to uint16_t

    /**
     * List of the bgp connections active when the application is running
     */
    std::vector<BGPConnection*> bgpConnections;

    std::vector<Socket*> listeningSockets;

    std::atomic<bool> running = {false};

    // BGP Routing Table

    std::string name = "BGPApp";

    /**
     * The router that started the application
     */
    Router* router = nullptr;


    BGPApplication(Router* router, pcpp::IPv4Address BGPIdentifier);
    ~BGPApplication();

    /**
     * The BGP application start without immediately connecting to the peers,
     * but waiting for the peer to connect
     */
    void passiveOpenAll();

    /**
     * The BGP application starts by actively connecting to the remote peers
     * // TODO implement it
     */
    void activeOpen();


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
    void           collisionDetection(BGPConnection* connectionToCheck);
    BGPConnection* createNewBgpConnection();
    Socket*        getCorrespondingListeningSocket(pcpp::IPv4Address srcAddress,
                                                   uint16_t          srcPort);

    pcpp::IPv4Address getBGPIdentifier() {return BGPIdentifier;}

};

#endif  // BGPSIMULATION_BGP_BGPAPPLICATION_H
