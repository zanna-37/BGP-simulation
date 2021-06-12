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

BGPApplication::BGPApplication(Router* router) : router(router) {}

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

void BGPApplication::passiveOpen() {
    running = true;
    // Foreach BGP peer in the BGP Routing table
    for (pcpp::IPv4Address peerAddr : router->peer_addresses) {
        NetworkCard* egressNetCard =
            router->getNextHopNetworkCardOrNull(peerAddr);

        if (egressNetCard == nullptr) {
            L_ERROR(router->ID,
                    peerAddr.toString() + ": Destination unreachable");
        } else {
            pcpp::IPv4Address srcAddr = egressNetCard->IP;

            // Pre-create the socketListen that will be referenced from the
            // BgpConnection
            auto* socketListen = new Socket(router);
            listeningSockets.push_back(socketListen);
            socketListen->bind(srcAddr, BGPApplication::BGPDefaultPort);

            auto* bgpConnection = new BGPConnection(router, this);
            bgpConnections.push_back(bgpConnection);
            bgpConnection->srcAddr = srcAddr;
            bgpConnection->srcPort = BGPApplication::BGPDefaultPort;
            bgpConnection->dstAddr = peerAddr;

            bgpConnection->enqueueEvent(
                BGPEvent::ManualStart_with_PassiveTcpEstablishment);
        }
    }
}

void BGPApplication::collisionDetection(BGPConnection* connectionToCheck) {
    for (BGPConnection* connection : bgpConnections) {
        if (connectionToCheck->dstAddr == connection->dstAddr &&
            connection != connectionToCheck) {
            if (connectionToCheck->srcAddr < connection->dstAddr) {
                connectionToCheck->enqueueEvent(BGPEvent::ManualStop);
            } else {
                connection->enqueueEvent(BGPEvent::ManualStop);
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
    for (auto* listeningSocket : listeningSockets) {
        // TODO check null ↓
        if (listeningSocket->tcpConnection->srcAddr == srcAddress &&
            listeningSocket->tcpConnection->srcPort == srcPort) {
            return listeningSocket;
        }
    }
    return nullptr;
}
