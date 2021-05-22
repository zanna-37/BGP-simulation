#include "BGPApplication.h"

#include <string>

#include "../ip/IpManager.h"
#include "../logger/Logger.h"
#include "BGPEvent.h"

BGPApplication::BGPApplication(Router* router) : router(router) {}

BGPApplication::~BGPApplication() {
    running = false;
    for (Socket* listeningSocket : listeningSockets) {
        listeningSocket->close();
    }
    for (thread& listeningThread : listeningThreads) {
        listeningThread.join();
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
    for (pcpp::IPv4Address peer : router->peer_addresses) {
        auto* connection = new BGPConnection(router);
        bgpConnections.push_back(connection);

        NetworkCard* egressNetCard =
            IpManager::findExitingNetworkCard(peer, router->routingTable);
        connection->srcAddr = egressNetCard->IP;  // Get egress IP
        connection->dstAddr = peer;

        connection->enqueueEvent(
            BGPEvent::ManualStart_with_PassiveTcpEstablishment);

        Socket* socketListen = router->getNewSocket(Socket::Domain::AF_INET,
                                                    Socket::Type::SOCK_STREAM);
        listeningSockets.push_back(socketListen);

        listeningThreads.emplace_back([&, connection, socketListen]() {
            socketListen->bind(connection->srcAddr,
                               BGPApplication::BGPDefaultPort);

            while (running) {
                // wait for the socket to have TCP connection pending
                socketListen->listen();

                if (running) {
                    Socket* tempSocket = socketListen->accept();
                    bindSocketToBGPConnection(tempSocket);

                } else {
                    L_DEBUG(name + " LTh", "Shutting Down");
                }
            }
        });
    }
}

void BGPApplication::bindSocketToBGPConnection(Socket* socket) {
    for (BGPConnection* connection : bgpConnections) {
        if (connection->srcAddr == socket->srcAddr &&
            connection->dstAddr == socket->dstAddr) {
            if (connection->connectedSocket == nullptr) {
                connection->connectedSocket = socket;
                connection->enqueueEvent(BGPEvent::TcpConnectionConfirmed);
                connection->receiveData();
            } else {
                BGPConnection* newBGPConnection = new BGPConnection(router);
                newBGPConnection->enqueueEvent(
                    BGPEvent::ManualStart_with_PassiveTcpEstablishment);
                newBGPConnection->srcAddr         = socket->srcAddr;
                newBGPConnection->dstAddr         = socket->dstAddr;
                newBGPConnection->connectedSocket = socket;
                newBGPConnection->enqueueEvent(
                    BGPEvent::TcpConnectionConfirmed);
                newBGPConnection->receiveData();
                bgpConnections.push_back(newBGPConnection);
            }
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