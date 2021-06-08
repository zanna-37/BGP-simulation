#include "Device.h"

#include <random>
#include <utility>

#include "../ip/IpManager.h"
#include "../ip/TableRow.h"
#include "../logger/Logger.h"
#include "../tcp/TCPConnection.h"
#include "../tcp/fsm/TCPState.h"
#include "../tcp/fsm/TCPStateMachine.h"
#include "IPv4Layer.h"
#include "Layer.h"
#include "NetworkCard.h"
#include "TcpLayer.h"


Device::Device(std::string ID, pcpp::IPv4Address defaultGateway)
    : ID(std::move(ID)), defaultGateway(defaultGateway) {}

NetworkCard *Device::getNetworkCardByInterfaceOrNull(
    const std::string &interfaceToSearch) const {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}

Device::~Device() {
    // Note: all the subclasses need to call Device::shutdown() in their
    // deconstructor. Keep them in sync.

    for (NetworkCard *networkCard : *networkCards) {
        networkCard->shutdown();
    }

    for (std::thread &netInputThread : netInputThreads) {
        netInputThread.join();
    }

    for (NetworkCard *networkCard : *networkCards) {
        delete networkCard;
    }
    delete networkCards;

    for (auto itr = tcpConnections.begin(); itr != tcpConnections.end();) {
        auto tcpConnection = itr->lock();

        if (tcpConnection) {
            L_ERROR("TCP",
                    tcpConnection->stateMachine->getCurrentState()->name +
                        " TCP connection not yet deallocated");
            itr++;
        } else {
            // Garbage collector
            itr = tcpConnections.erase(itr);
        }
    }
    L_SUCCESS(ID, "Shutdown completed");
}

void Device::addCards(std::vector<NetworkCard *> *networkCards) {
    this->networkCards = networkCards;
}

void Device::bootUp() {
    L_VERBOSE(ID, "Booting up...");

    IpManager::buildRoutingTable(routingTable, networkCards, defaultGateway);

    std::string routingTableAsString =
        IpManager::getRoutingTableAsString(routingTable);
    L_VERBOSE(ID, "Routing table:\n" + routingTableAsString);

    running = true;

    for (NetworkCard *networkCard : *networkCards) {
        netInputThreads.emplace_back([&, networkCard]() {
            while (running) {
                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers = networkCard->waitForL3Packet();

                if (running && layers) {
                    auto *ipLayer_weak =
                        dynamic_cast<pcpp::IPv4Layer *>(layers->top().get());
                    pcpp::IPv4Address dstAddress =
                        ipLayer_weak->getDstIPv4Address();
                    ipLayer_weak = nullptr;


                    std::string logMessage = "Received packet from " +
                                             networkCard->netInterface + ": ";
                    if (dstAddress == networkCard->IP) {
                        L_DEBUG(ID,
                                logMessage + "input chain, processing message");
                        processMessage(std::move(layers));
                    } else {
                        L_DEBUG(
                            ID,
                            logMessage + "forward chain, forwarding message");
                        NetworkCard *nextHopNetworkCard =
                            IpManager::findExitingNetworkCard(dstAddress,
                                                              routingTable);
                        if (nextHopNetworkCard == nullptr) {
                            L_ERROR(ID,
                                    dstAddress.toString() +
                                        ": Destination unreachable");
                        } else {
                            forwardMessage(std::move(layers),
                                           nextHopNetworkCard);
                        }
                    }
                }
            }
        });
    }

    bootUpInternal();
    L_SUCCESS(ID, "Booting up completed");
}

void Device::sendPacket(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers,
    const pcpp::IPv4Address &                                 dstAddr) {
    auto ipLayer = std::make_unique<pcpp::IPv4Layer>();
    ipLayer->setDstIPv4Address(dstAddr);

    NetworkCard *nextHopNetworkCard = getNextHopNetworkCardOrNull(dstAddr);

    if (nextHopNetworkCard == nullptr) {
        L_ERROR(ID, dstAddr.toString() + ": Destination unreachable");
    } else {
        L_DEBUG(ID, "Sending packet using " + nextHopNetworkCard->netInterface);
        ipLayer->setSrcIPv4Address(nextHopNetworkCard->IP);
        layers->push(std::move(ipLayer));
        nextHopNetworkCard->sendPacket(std::move(layers));
    }
}
NetworkCard *Device::getNextHopNetworkCardOrNull(
    const pcpp::IPv4Address &dstAddr) const {
    NetworkCard *nextHopNetworkCard =
        IpManager::findExitingNetworkCard(dstAddr, routingTable);
    return nextHopNetworkCard;
}

void Device::processMessage(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    auto ipLayer = std::move(layers->top());
    layers->pop();
    auto tcpLayer = std::move(layers->top());
    layers->pop();

    auto *ipLayer_weak  = dynamic_cast<pcpp::IPv4Layer *>(ipLayer.get());
    auto *tcpLayer_weak = dynamic_cast<pcpp::TcpLayer *>(tcpLayer.get());

    layers->push(std::move(tcpLayer));

    std::shared_ptr<TCPConnection> tcpConnection = nullptr;

    // Check if the message is part of an existing connection
    tcpConnection =
        getExistingTcpConnectionOrNull(ipLayer_weak->getDstIPv4Address(),
                                       tcpLayer_weak->getDstPort(),
                                       ipLayer_weak->getSrcIPv4Address(),
                                       tcpLayer_weak->getSrcPort());


    if (tcpConnection != nullptr) {
        tcpConnection->segmentArrives(std::make_pair(
            ipLayer_weak->getSrcIPv4Address(), std::move(layers)));
    } else {
        L_ERROR(ID,
                "No TCP service listening on " +
                    ipLayer_weak->getDstIPv4Address().toString() + " port " +
                    std::to_string(tcpLayer_weak->getDstPort()));

        // TODO send reset
    }
}

std::shared_ptr<TCPConnection> Device::getExistingTcpConnectionOrNull(
    const pcpp::IPv4Address &srcAddr,
    const uint16_t &         srcPort,
    const pcpp::IPv4Address &dstAddr,
    const uint16_t &         dstPort) {
    std::shared_ptr<TCPConnection> connectedTcpConn;
    std::shared_ptr<TCPConnection> listeningTcpConn;

    for (auto itr = tcpConnections.begin(); itr != tcpConnections.end();) {
        auto tcpConnection = itr->lock();

        if (tcpConnection) {
            if (tcpConnection->srcAddr == srcAddr &&
                tcpConnection->srcPort == srcPort &&
                tcpConnection->dstAddr == dstAddr &&
                tcpConnection->dstPort == dstPort) {
                // Return an existing connection
                connectedTcpConn = tcpConnection;
                break;
            } else if (tcpConnection->srcAddr == srcAddr &&
                       tcpConnection->srcPort == srcPort) {
                // Save the listening connection in case we don't find a more
                // specific established TCP connection
                listeningTcpConn = tcpConnection;
            }

            itr++;
        } else {
            // Garbage collector
            itr = tcpConnections.erase(itr);
        }
    }

    if (connectedTcpConn) {
        return connectedTcpConn;
    } else if (listeningTcpConn) {
        return listeningTcpConn;
    } else {
        return nullptr;
    }
}

int Device::bind(const std::shared_ptr<TCPConnection> &tcpConnection) {
    std::unique_lock<std::mutex> ports_uniqueLock(ports_mutex, std::defer_lock);

    if (tcpConnection->srcPort == 0) {
        ports_uniqueLock.lock();
        tcpConnection->srcPort = getFreePort();
    }

    // TODO check if the addr/port are really free
    bool alreadyInUse = false;

    if (alreadyInUse == false) {
        tcpConnections.push_back(tcpConnection);
        return 0;
    } else {
        return 1;
    }
}
uint16_t Device::getFreePort() /* guarded_by ports_mutex */ {
    std::random_device
        rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd());  // Standard mersenne_twister_engine seeded with
                             // rd()
    std::uniform_int_distribution<> distrib(
        49152, 65535);  // TODO test if ports are really free

    return (uint16_t)distrib(gen);
}

void Device::shutdown() {
    L_VERBOSE(ID, "Shutting down...");
    running = false;
}
