#include "BGPApplication.h"

#include <string>

#include "../logger/Logger.h"
#include "BGPEvent.h"

BGPApplication::BGPApplication(Router* router) : router(router) {}

BGPApplication::~BGPApplication() {
    delete applicationThread;

    for (BGPConnection* connection : bgpConnections) {
        delete connection;
    }
}


void BGPApplication::passiveOpen() {
    running = true;
    /*each BGP peer in the BGP Rounting table*/
    for (pcpp::IPv4Address peer : router->peerList) {
        BGPConnection* connection = new BGPConnection(router);

        connection->srcAddr = router->findNextHop(peer)->IP;
        connection->dstAddr = peer;

        bgpConnections.push_back(connection);

        connection->enqueueEvent(
            BGPEvent::ManualStart_with_PassiveTcpEstablishment);


        std::thread* listeningThread = new std::thread([&]() {
            Socket* socketListen = router->getNewSocket(
                Socket::Domain::AF_INET, Socket::Type::SOCK_STREAM);
            socketListen->bind(connection->srcAddr, 179);

            while (running) {
                // wait for the socket to have TCP connection pending
                socketListen->listen();

                if (running) {
                    Socket* tempSocket = socketListen->accept();
                    bindSocketToBGPConnection(tempSocket);

                } else {
                    L_DEBUG(name, "Shutting Down");
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