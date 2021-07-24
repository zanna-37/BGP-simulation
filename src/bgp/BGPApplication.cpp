#include "BGPApplication.h"

#include <memory>
#include <string>

#include "../entities/NetworkCard.h"
#include "../entities/Router.h"
#include "../logger/Logger.h"
#include "../socket/Socket.h"
#include "../tcp/TCPConnection.h"
#include "BGPConnection.h"
#include "BGPEvent.h"

BGPApplication::BGPApplication(Router* router, pcpp::IPv4Address BGPIdentifier)
    : router(router), BGPIdentifier(BGPIdentifier) {}

BGPApplication::~BGPApplication() {
    running = false;

    for (BGPConnection* connection : bgpConnections) {
        connection->shutdown();
    }
    for (Socket* listeningSocket : listeningSockets) {
        listeningSocket->close();
    }
    for (BGPConnection* connection : bgpConnections) {
        delete connection;
    }
    for (Socket* listeningSocket : listeningSockets) {
        delete listeningSocket;
    }
}

void BGPApplication::passiveOpenAll() {
    running = true;
    // Foreach BGP peer in the BGP Routing table
    for (pcpp::IPv4Address peerAddr : router->peer_addresses) {
        NetworkCard* egressNetCard =
            router->getNextHopNetworkCardOrNull(peerAddr);

        if (egressNetCard == nullptr) {
            L_ERROR(router->ID + " BGP App",
                    peerAddr.toString() + ": Destination unreachable");
        } else {
            pcpp::IPv4Address srcAddr = egressNetCard->IP;

            auto* bgpConnection = new BGPConnection(router, this);
            bgpConnections.push_back(bgpConnection);
            bgpConnection->srcAddr = srcAddr;
            bgpConnection->srcPort = BGPApplication::BGPDefaultPort;
            bgpConnection->dstAddr = peerAddr;

            BGPEvent event = {
                BGPEventList::ManualStart_with_PassiveTcpEstablishment,
                nullptr,
            };

            bgpConnection->enqueueEvent(event);
        }
    }
}

void BGPApplication::collisionDetection(BGPConnection* connectionToCheck) {
    for (BGPConnection* connection : bgpConnections) {
        if (connectionToCheck->dstAddr == connection->dstAddr &&
            connection != connectionToCheck) {
            BGPEvent event = {
                BGPEventList::ManualStop,
                nullptr,
            };
            if (connectionToCheck->srcAddr < connection->dstAddr) {
                connectionToCheck->enqueueEvent(event);
            } else {
                connection->enqueueEvent(event);
            }
        }
    }
}
BGPConnection* BGPApplication::createNewBgpConnection() {
    auto* newBGPConnection = new BGPConnection(router, this);
    bgpConnections.push_back(newBGPConnection);
    return newBGPConnection;
}

Socket* BGPApplication::getCorrespondingListeningSocket(
    pcpp::IPv4Address srcAddress, uint16_t srcPort) {
    std::mutex mutex;
    mutex.lock();
    Socket* result;
    // Search existing listening socket
    for (auto* listeningSocket : listeningSockets) {
        if (listeningSocket->tcpConnection->srcAddr == srcAddress &&
            listeningSocket->tcpConnection->srcPort == srcPort) {
            result = listeningSocket;
            L_DEBUG(router->ID + " BGP App",
                    "Reusing listening Socket at " + srcAddress.toString() +
                        ":" + std::to_string(srcPort));
        }
    }
    if (result == nullptr) {
        // Create a new listening socket if it doesn't already exist
        L_DEBUG(router->ID + " BGP App",
                "Creating new listening Socket listen at " +
                    srcAddress.toString() + ":" + std::to_string(srcPort));
        auto* newSocketListen = new Socket(router);
        listeningSockets.push_back(newSocketListen);
        newSocketListen->bind(srcAddress, BGPApplication::BGPDefaultPort);
        result = newSocketListen;
    }
    mutex.unlock();

    return result;
}
