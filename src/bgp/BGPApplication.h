#ifndef BGPSIMULATION_BGP_BGPAPPLICATION_H
#define BGPSIMULATION_BGP_BGPAPPLICATION_H

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "../socket/Socket.h"
#include "IpAddress.h"

// forward declarations
#include "../entities/Router.fwd.h"
#include "BGPApplication.fwd.h"
#include "./packets/BGPUpdateLayer.h"
#include "./packets/BGPUpdateLengthAndIpPrefix.h"
#include "./packets/BGPUpdatePathAttribute.h"
#include "BGPConnection.fwd.h"


class ListeningSocketModule {
   private:
    BGPApplication*   bgpApp;
    Socket*           listeningSocket;
    std::thread*      listeningSocketsThread;
    std::atomic<bool> running = {false};

   public:
    Socket* getSocket();

    ListeningSocketModule(Socket* socket, BGPApplication* BGPAppParent);
    ~ListeningSocketModule();

    void startListeningThread(pcpp::IPv4Address srcAddress);
    void stopListeningThread();
};

class BGPApplication {
   private:
    // The value of the BGP Identifier is determined upon
    // startup and is the same for every local interface and BGP peer.
    // TODO define if we want to leave the first network card as bgpIdentifier
    pcpp::IPv4Address BGPIdentifier = nullptr;


    std::mutex                          listeningSockets_mutex;
    std::vector<ListeningSocketModule*> listeningSocketModules;

    ListeningSocketModule* getOrCreateCorrespondingListeningSocketModule(
        pcpp::IPv4Address srcAddress, uint16_t srcPort);

   protected:
    /**
     * List of the bgp connections active when the application is running
     */
    std::vector<BGPConnection*> bgpConnections;

   public:
    static const int BGPDefaultPort = 179;  // TODO change to uint16_t will do

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
    BGPConnection* createNewBgpConnection(pcpp::IPv4Address srcAddress,
                                          pcpp::IPv4Address dstAddress);

    pcpp::IPv4Address getBGPIdentifier() { return BGPIdentifier; }

    void startListeningOnSocket(pcpp::IPv4Address srcAddress);

    void stopListeningOnSocket(pcpp::IPv4Address srcAddress);

    BGPConnection* setConnectedSocketToAvailableBGPConn(
        Socket*           newConnectedSocket,
        pcpp::IPv4Address bgpConnectionSrcAddressToBindTo,
        pcpp::IPv4Address bgpConnectionDstAddressToBindTo);

    void sendBGPUpdateMessage(BGPConnection* bgpConnectionToAvoid,
                              std::vector<LengthAndIpPrefix> withdrawnroutes,
                              std::vector<PathAttribute>     pathAttributes,
                              std::vector<LengthAndIpPrefix> nlri);

    friend class ListeningSocketModule;
};

#endif  // BGPSIMULATION_BGP_BGPAPPLICATION_H
