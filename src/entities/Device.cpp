#include "Device.h"

#include <iomanip>
#include <iostream>

#include "../bgp/BGPEvent.h"
#include "../bgp/packets/BGPLayer.h"
#include "../ip/IpManager.h"
#include "../logger/Logger.h"

Device::Device(string ID, pcpp::IPv4Address defaultGateway)
    : ID(std::move(ID)), defaultGateway(defaultGateway) {}

NetworkCard *Device::getNetworkCardByInterfaceOrNull(
    const string &interfaceToSearch) const {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}

Device::~Device() {
    L_VERBOSE(ID, "Shutting down");
    running = false;

    unique_lock<std::mutex> receivedPacketsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);
    receivedPacketsEventQueue_wakeup.notify_one();
    receivedPacketsEventQueue_uniqueLock.unlock();
    deviceThread->join();
    delete deviceThread;

    for (NetworkCard *networkCard : *networkCards) {
        delete networkCard;
    }
    delete networkCards;

    delete listenConnection;

    // for (BGPConnection *connection : bgpConnections) {
    //     delete connection;
    // }

    // std::map<std::size_t, TCPConnection *>::iterator it;
    // for (it = tcpConnections.begin(); it != tcpConnections.end(); ++it) {
    //     delete it->second;
    // }

    for (TCPConnection *connection : tcpConnections) {
        delete connection;
    }
}

void Device::addCards(vector<NetworkCard *> *networkCards) {
    this->networkCards = networkCards;
}

void Device::bootUp() {
    L_VERBOSE(ID, "Booting up");

    IpManager::buildRoutingTable(routingTable, networkCards, defaultGateway);

    string routingTableAsString =
        IpManager::getRoutingTableAsString(routingTable);
    L_VERBOSE(ID, "Routing table:\n" + routingTableAsString);

    running      = true;
    deviceThread = new std::thread([&]() {
        while (running) {
            unique_lock<std::mutex> receivedPacketsEventQueue_uniqueLock(
                receivedPacketsEventQueue_mutex);

            while (receivedPacketsEventQueue.empty() && running) {
                receivedPacketsEventQueue_wakeup.wait(
                    receivedPacketsEventQueue_uniqueLock);

                if (receivedPacketsEventQueue.empty() && running) {
                    L_DEBUG(ID, "Spurious Wakeup");
                }
            }

            if (running) {
                ReceivedPacketEvent *event = receivedPacketsEventQueue.front();
                L_DEBUG(ID,
                        "Packet arrived through " +
                            event->networkCard->netInterface);
                receivedPacketsEventQueue.pop();
                receivedPacketsEventQueue_uniqueLock.unlock();

                event->networkCard->handleNextPacket();
                delete event;
            } else {
                receivedPacketsEventQueue_uniqueLock.unlock();
            }
        }
    });
}

void Device::sendPacket(stack<pcpp::Layer *> *   layers,
                        const pcpp::IPv4Address &dstAddr) {
    auto *ipLayer = new pcpp::IPv4Layer();
    ipLayer->setDstIPv4Address(dstAddr);

    NetworkCard *nextHopNetworkCard =
        IpManager::findExitingNetworkCard(dstAddr, routingTable);
    if (nextHopNetworkCard == nullptr) {
        L_ERROR(ID, dstAddr.toString() + ": Destination unreachable");
        // FIXME
        delete ipLayer;

    } else {
        L_DEBUG(ID, "Sending packet using " + nextHopNetworkCard->netInterface);
        ipLayer->setSrcIPv4Address(nextHopNetworkCard->IP);
        // complete TCP connection details with srcAddr
        for (TCPConnection *connection : tcpConnections) {
            if (connection->srcAddr == pcpp::IPv4Address::Zero &&
                connection->dstAddr == dstAddr) {
                connection->srcAddr = nextHopNetworkCard->IP;
            }
        }
        layers->push(ipLayer);
        nextHopNetworkCard->sendPacket(layers);
    }
}

void Device::receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    string logMessage = "Received packet from " + origin->netInterface + ": ";
    if (dstAddress == origin->IP) {
        L_DEBUG(ID, logMessage + "input chain, processing message");
        processMessage(layers);
    } else {
        L_DEBUG(ID, logMessage + "forward chain, forwarding message");
        NetworkCard *nextHopNetworkCard =
            IpManager::findExitingNetworkCard(dstAddress, routingTable);
        if (nextHopNetworkCard == nullptr) {
            L_ERROR(ID, dstAddress.toString() + ": Destination unreachable");
        } else {
            forwardMessage(layers, nextHopNetworkCard);
        }
    }
}

void Device::processMessage(stack<pcpp::Layer *> *layers) {
    pcpp::IPv4Layer *ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    layers->pop();
    pcpp::TcpLayer *tcpLayer = dynamic_cast<pcpp::TcpLayer *>(layers->top());
    layers->push(ipLayer);

    TCPConnection *existingTcpConnection =
        getExistingTcpConnectionOrNull(ipLayer->getSrcIPv4Address(),
                                       tcpLayer->getSrcPort(),
                                       ipLayer->getDstIPv4Address(),
                                       tcpLayer->getDstPort());
    if (existingTcpConnection != nullptr) {
        existingTcpConnection->processMessage(layers);
    } else {
        L_ERROR(ID,
                "No TCP service listening on " +
                    ipLayer->getDstIPv4Address().toString() + " port " +
                    to_string(tcpLayer->getDstPort()));
    }
}

void Device::enqueueEvent(ReceivedPacketEvent *event) {
    L_DEBUG(ID, "Enqueueing event in receivedPacketEventQueue");
    unique_lock<std::mutex> receivedPacketsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);

    receivedPacketsEventQueue.push(event);
    receivedPacketsEventQueue_wakeup.notify_one();

    receivedPacketsEventQueue_uniqueLock.unlock();
}

void Device::listen(uint16_t port) {
    // Initialize passive Open Listen for TCP
    listenConnection          = new TCPConnection(this);
    listenConnection->srcPort = port;
    listenConnection->enqueueEvent(TCPEvent::PassiveOpen);
}
// FIXME
// void Device::resetConnection(std::string dstAddr, uint16_t dstPort) {
//     TCPConnection *existingConnection =
//         getExistingTcpConnectionOrNull(dstAddr, dstPort);

//     if (existingConnection == nullptr) {
//         L_ERROR(ID, "Trying to close an non existing connection");
//     } else {
//         existingConnection->enqueueEvent(TCPEvent::SendRST);
//     }
// }

TCPConnection *Device::getExistingTcpConnectionOrNull(
    const pcpp::IPv4Address &srcAddr,
    uint16_t                 srcPort,
    const pcpp::IPv4Address &dstAddr,
    uint16_t                 dstPort) {
    for (TCPConnection *connection : tcpConnections) {
        // garbage collector for already closed TCP connection
        if (connection->stateMachine->getCurrentState()->name == "CLOSED") {
            delete connection;
            return nullptr;
        }

        if (connection->srcAddr == srcAddr && connection->srcPort == srcPort &&
            connection->dstAddr == dstAddr && connection->dstPort == dstPort) {
            return connection;
        }
        // new TCP connection with unknown remote addresses
        else if (connection->srcAddr == srcAddr &&
                 connection->srcPort == srcPort &&
                 connection->dstAddr == pcpp::IPv4Address::Zero &&
                 connection->dstPort == 0) {
            connection->dstAddr = dstAddr;
            connection->dstPort = dstPort;
            return connection;
        }
    }
    return nullptr;
}
// TODO modify the hash so that we are sure we will not have a collision
// TODO be sure that addTCPConnection(...) do not overwrite another existing
// connection with the same address and port
// void Device::addTCPConnection(TCPConnection *connection) {
//     tcpConnections[tcpConnectionHash(connection->dstAddr->toString(),
//                                      connection->dstPort)] = connection;
// }

// void Device::removeTCPConnection(TCPConnection *connection) {
//     tcpConnections.erase(tcpConnectionHash(connection->dstAddr->toString(),
//                                            connection->dstPort));
// }


// std::size_t Device::tcpConnectionHash(std::string dstAddr, uint16_t dstPort)
// {
//     size_t returnHash;

//     size_t hash1 = std::hash<std::string>{}(dstAddr);
//     size_t hash2 = std::hash<std::string>{}(to_string(dstPort));

//     return hash1 ^ hash2;
// }


// void Device::handleApplicationLayer(std::stack<pcpp::Layer *> *layers,
//                                     TCPConnection *            tcpConnection)
//                                     {
//     // the device will call the proper connection callback

//     if (tcpConnection->srcPort == 179) {
//         for (BGPConnection *connection : bgpConnections) {
//             if (connection->tcpConnection == tcpConnection) {
//                 connection->processMessage(layers);
//                 return;
//             }
//         }

//         // handle new BGP connection
//     }
// }

// void Device::bgpConnect(std::string dstAddr) {
//     BGPConnection *connection = new BGPConnection(this);
//     connection->dstAddr       = dstAddr;

//     connection->enqueueEvent(BGPEvent::ManualStart);

//     bgpConnections.push_back(connection);
// }

// void Device::connectionConfirmed(TCPConnection *tcpConnection) {
//     if (tcpConnection->srcPort == 179) {
//         BGPConnection *connection = findBGPConnectionOrNull(tcpConnection);
//         if (connection != nullptr) {
//             connection->dstAddr = tcpConnection->srcAddr->toString();
//             connection->enqueueEvent(BGPEvent::TcpConnectionConfirmed);
//         }
//     }
// }

// void Device::connectionAcked(TCPConnection *tcpConnection) {
//     if (tcpConnection->dstPort == 179) {
//         BGPConnection *bgpConnection =
//         findBGPConnectionOrNull(tcpConnection); if (bgpConnection != nullptr)
//         {
//             bgpConnection->enqueueEvent(BGPEvent::Tcp_CR_Acked);
//         }
//     }
// }

// BGPConnection *Device::findBGPConnectionOrNull(TCPConnection *tcpConnection)
// {
//     for (BGPConnection *connection : bgpConnections) {
//         if (connection->tcpConnection == tcpConnection) {
//             return connection;
//         }
//     }
//     return nullptr;
// }

// void Device::bgpListen() {
//     BGPConnection *connection = new BGPConnection(this);

//     connection->enqueueEvent(
//         BGPEvent::ManualStart_with_PassiveTcpEstablishment);

//     bgpConnections.push_back(connection);
// }

// void Device::tcpConnectionClosed(TCPConnection *tcpConnection) {
//     if (tcpConnection->dstPort == 179) {
//         BGPConnection *bgpConnection =
//         findBGPConnectionOrNull(tcpConnection); if (bgpConnection != nullptr)
//         {
//             bgpConnection->enqueueEvent(BGPEvent::TcpConnectionFails);
//         }
//     }
// }

Socket *Device::getNewSocket(int type, int domain) {
    auto *socket = new Socket(type, domain);

    socket->device = this;
    listeningSockets.push_back(socket);

    return socket;
}


Socket *Device::getAssociatedListeningSocketOrNull(
    TCPConnection *tcpConnection) {
    for (Socket *s : listeningSockets) {
        if (tcpConnection->srcAddr == s->srcAddr &&
            tcpConnection->srcPort == s->srcPort) {
            return s;
        }
    }
    return nullptr;
}

Socket *Device::getAssociatedConnectedSocketOrNull(
    TCPConnection *tcpConnection) {
    for (Socket *s : connectedSockets) {
        if (tcpConnection->srcAddr == s->srcAddr &&
            tcpConnection->srcPort == s->srcPort &&
            tcpConnection->dstAddr == s->dstAddr &&
            tcpConnection->dstPort == s->dstPort) {
            return s;
        }
    }
    return nullptr;
}

TCPConnection *Device::getAssociatedTCPconnectionOrNull(Socket *socket) {
    for (TCPConnection *c : tcpConnections) {
        if (c->srcAddr == socket->srcAddr && c->srcPort == socket->srcPort &&
            c->dstAddr == socket->dstAddr && c->dstPort == socket->dstPort) {
            return c;
        }
    }

    return nullptr;
}

TCPConnection *Device::getNewTCPConnection(const pcpp::IPv4Address &srcAddr,
                                           uint16_t                 srcPort) {
    auto *connection = new TCPConnection(this);

    connection->srcAddr = srcAddr;
    connection->srcPort = srcPort;

    tcpConnections.push_back(connection);

    return connection;
}

void Device::notifyListeningSocket(TCPConnection *connection) {
    Socket *s = getAssociatedListeningSocketOrNull(connection);
    if (s != nullptr) {
        s->dataArrived();
    } else {
        L_FATAL(ID, "No listening socket associated with TCP connection");
    }
}

void Device::notifyConnectedSocket(TCPConnection *connection) {
    Socket *s = getAssociatedConnectedSocketOrNull(connection);
    if (s != nullptr) {
        s->dataArrived();
    } else {
        L_FATAL(ID, "No connected socket associated with TCP connection");
    }
}

// ### ReceivedPacketEvent methods

ReceivedPacketEvent::ReceivedPacketEvent(NetworkCard *networkCard,
                                         Description  description)
    : networkCard(networkCard), description(description) {}
