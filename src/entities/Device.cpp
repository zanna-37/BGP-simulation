#include "Device.h"

#include <iomanip>
#include <iostream>

#include "../bgp/BGPEvent.h"
#include "../bgp/packets/BGPLayer.h"
#include "../logger/Logger.h"

Device::Device(string ID, pcpp::IPv4Address defaultGateway)
    : ID(std::move(ID)), defaultGateway(defaultGateway) {}

NetworkCard *Device::getNetworkCardByInterfaceOrNull(

    const string &interfaceToSearch) {
    for (const auto &networkCard : *networkCards) {
        if (networkCard->netInterface == interfaceToSearch) {
            return networkCard;
        }
    }
    return nullptr;
}

Device::~Device() {
    running = false;

    unique_lock<std::mutex> receivedPacketsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);
    receivedPacketsEventQueue_wakeup.notify_one();
    receivedPacketsEventQueue_uniqueLock.unlock();
    deviceThread->join();
    delete deviceThread;

    for (TableRow *row : *routingTable) {
        delete row;
    }
    delete routingTable;
    for (NetworkCard *networkCard : *networkCards) {
        delete networkCard;
    }
    delete networkCards;

    delete listenConnection;

    for (BGPConnection *connection : bgpConnections) {
        delete connection;
    }

    std::map<std::size_t, TCPConnection *>::iterator it;
    for (it = tcpConnections.begin(); it != tcpConnections.end(); ++it) {
        delete it->second;
    }
}

void Device::addCards(vector<NetworkCard *> *networkCards) {
    this->networkCards = networkCards;
}


void Device::start() {
    routingTable = new std::vector<TableRow *>();
    for (NetworkCard *networkCard : *networkCards) {
        pcpp::IPv4Address networkIP(networkCard->IP.toInt() &
                                    networkCard->netmask.toInt());

        TableRow *row = new TableRow(networkIP,
                                     pcpp::IPv4Address(networkCard->netmask),
                                     pcpp::IPv4Address::Zero,
                                     networkCard->netInterface,
                                     networkCard);

        routingTable->push_back(row);

        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(networkIP, networkCard->netmask)) {
            TableRow *row = new TableRow(pcpp::IPv4Address::Zero,
                                         pcpp::IPv4Address::Zero,
                                         pcpp::IPv4Address(defaultGateway),
                                         networkCard->netInterface,
                                         networkCard);

            routingTable->push_back(row);
        }
    }

    printTable();

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
                L_DEBUG(ID, "Queue not empty: handling event");
                ReceivedPacketEvent *event = receivedPacketsEventQueue.front();
                L_DEBUG(
                    ID,
                    "Packet arrived at " + event->networkCard->netInterface);
                receivedPacketsEventQueue.pop();

                event->networkCard->handleNextPacket();

                delete event;
            } else {
                L_VERBOSE(ID, "Shutting down");
            }
            receivedPacketsEventQueue_uniqueLock.unlock();
        }
    });
}

void Device::sendPacket(stack<pcpp::Layer *> *layers, std::string dstAddr_str) {
    pcpp::IPv4Layer *ipLayer = new pcpp::IPv4Layer();
    ipLayer->setDstIPv4Address(pcpp::IPv4Address(dstAddr_str));
    pcpp::IPv4Address dstAddr = ipLayer->getDstIPv4Address();

    NetworkCard *nextHopNetworkCard = findNextHop(&dstAddr);
    if (nextHopNetworkCard == nullptr) {
        L_ERROR(ID, "DESTINATION UNREACHABLE");
        // FIXME
        delete ipLayer;

    } else {
        L_DEBUG(ID, "Sending packet using " + nextHopNetworkCard->netInterface);
        ipLayer->setSrcIPv4Address(nextHopNetworkCard->IP);
        layers->push(ipLayer);
        nextHopNetworkCard->sendPacket(layers);
    }
}

void Device::receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    if (dstAddress == origin->IP) {
        L_DEBUG(ID, "Processing message");
        processMessage(layers);
    } else {
        NetworkCard *nextHopNetworkCard = findNextHop(&dstAddress);
        if (nextHopNetworkCard == nullptr) {
            L_ERROR(ID, "DESTINATION UNREACHABLE");
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

    TCPConnection *existingConnection = getExistingConnectionOrNull(
        ipLayer->getSrcIPv4Address().toString(), tcpLayer->getSrcPort());
    if (existingConnection != nullptr) {
        L_DEBUG(ID, "Existing connection");
        existingConnection->processMessage(layers);
    } else {
        if (listenConnection != nullptr &&
            listenConnection->srcPort == tcpLayer->getDstPort()) {
            L_DEBUG(ID, "Handling new TCP Connection");
            // TODO not clear that we are creating a new connection with
            // listenConnection
            listenConnection->processMessage(layers);
        } else {
            L_INFO(ID, "PORT closed or server not listening");
            resetConnection(ipLayer->getSrcIPv4Address().toString(),
                            tcpLayer->getSrcPort());
        }
    }
}

void Device::enqueueEvent(ReceivedPacketEvent *event) {
    L_DEBUG(ID, "Enqueueing event");
    unique_lock<std::mutex> receivedPacketsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);

    receivedPacketsEventQueue.push(event);
    receivedPacketsEventQueue_wakeup.notify_one();

    receivedPacketsEventQueue_uniqueLock.unlock();
}

void Device::listen() {
    // Initialize passive Open Listen for TCP
    listenConnection          = new TCPConnection(this);
    listenConnection->srcPort = listenConnection->BGPPort;
    listenConnection->enqueueEvent(TCPEvent::PassiveOpen);
}
TCPConnection *Device::connect(std::string dstAddr, uint16_t dstPort) {
    TCPConnection *connection = new TCPConnection(this);

    connection->dstAddr = new pcpp::IPv4Address(dstAddr);
    connection->dstPort = dstPort;

    // FIXME
    uint16_t randomPort = 12345;
    connection->srcPort = randomPort;
    addTCPConnection(connection);
    connection->enqueueEvent(TCPEvent::ActiveOpen_SendSYN);

    return connection;
}

void Device::closeConnection(std::string dstAddr, uint16_t dstPort) {
    TCPConnection *existingConnection =
        getExistingConnectionOrNull(dstAddr, dstPort);

    if (existingConnection == nullptr) {
        L_ERROR(ID, "Trying to close an non existing connection");
    } else {
        existingConnection->enqueueEvent(TCPEvent::CloseSendFIN);
    }
}

void Device::resetConnection(std::string dstAddr, uint16_t dstPort) {
    TCPConnection *existingConnection =
        getExistingConnectionOrNull(dstAddr, dstPort);

    if (existingConnection == nullptr) {
        L_ERROR(ID, "Trying to close an non existing connection");
    } else {
        existingConnection->enqueueEvent(TCPEvent::SendRST);
    }
}

TCPConnection *Device::getExistingConnectionOrNull(std::string address,
                                                   uint16_t    port) {
    auto search = tcpConnections.find(tcpConnectionHash(address, port));

    if (search != tcpConnections.end()) {
        // The connection exists inside the hashmap but it was already closed
        // before, so we delete it and we create a new one
        if (search->second->stateMachine->getCurrentState()->name == "CLOSED") {
            TCPConnection *to_remove = search->second;
            removeTCPConnection(search->second);
            delete to_remove;
            return nullptr;
        } else {
            return search->second;
        }
    }

    return nullptr;
}
// TODO modify the hash so that we are sure we will not have a collision
// TODO be sure that addTCPConnection(...) do not overwrite another existing
// connection with the same address and port
void Device::addTCPConnection(TCPConnection *connection) {
    tcpConnections[tcpConnectionHash(connection->dstAddr->toString(),
                                     connection->dstPort)] = connection;
}

void Device::removeTCPConnection(TCPConnection *connection) {
    tcpConnections.erase(tcpConnectionHash(connection->dstAddr->toString(),
                                           connection->dstPort));
}


std::size_t Device::tcpConnectionHash(std::string dstAddr, uint16_t dstPort) {
    size_t returnHash;

    size_t hash1 = std::hash<std::string>{}(dstAddr);
    size_t hash2 = std::hash<std::string>{}(to_string(dstPort));

    return hash1 ^ hash2;
}

NetworkCard *Device::findNextHop(pcpp::IPv4Address *dstAddress) {
    int          longestMatch = -1;
    NetworkCard *result       = nullptr;
    for (TableRow *row : *routingTable) {
        if (dstAddress->matchSubnet(row->networkIP, row->netmask) &&
            row->toCIDR() > longestMatch) {
            longestMatch = row->toCIDR();
            result       = row->networkCard;
        }
    }

    return result;
}

void Device::printElement(std::string t) {
    const char separator = ' ';
    const int  width     = 16;
    std::cout << left << setw(width) << setfill(separator) << t;
}

void Device::printTable() {
    printElement("Destination");
    printElement("Gateway");
    printElement("Genmask");
    printElement("Iface");
    std::cout << std::endl;
    for (TableRow *row : *routingTable) {
        printElement(row->networkIP.toString());
        printElement(row->defaultGateway.toString());
        printElement(row->netmask.toString());
        printElement(row->netInterface);
        cout << std::endl;
    }
    std::cout << std::endl;
}


void Device::handleApplicationLayer(std::stack<pcpp::Layer *> *layers,
                                    TCPConnection *            tcpConnection) {
    // the device will call the proper connection callback

    if (tcpConnection->srcPort == 179) {
        for (BGPConnection *connection : bgpConnections) {
            if (connection->tcpConnection == tcpConnection) {
                connection->processMessage(layers);
                return;
            }
        }

        // handle new BGP connection
    }
}

void Device::bgpConnect(std::string dstAddr) {
    BGPConnection *connection = new BGPConnection(this);
    connection->dstAddr       = dstAddr;

    connection->enqueueEvent(BGPEvent::ManualStart);

    bgpConnections.push_back(connection);
}

void Device::connectionConfirmed(TCPConnection *tcpConnection) {
    if (tcpConnection->srcPort == 179) {
        BGPConnection *connection = findBGPConnectionOrNull(tcpConnection);
        if (connection != nullptr) {
            connection->dstAddr = tcpConnection->dstAddr->toString();
            connection->enqueueEvent(BGPEvent::TcpConnectionConfirmed);
        }
    }
}

void Device::connectionAcked(TCPConnection *tcpConnection) {
    if (tcpConnection->dstPort == 179) {
        BGPConnection *bgpConnection = findBGPConnectionOrNull(tcpConnection);
        if (bgpConnection != nullptr) {
            bgpConnection->enqueueEvent(BGPEvent::Tcp_CR_Acked);
        }
    }
}

BGPConnection *Device::findBGPConnectionOrNull(TCPConnection *tcpConnection) {
    for (BGPConnection *connection : bgpConnections) {
        if (connection->tcpConnection == tcpConnection) {
            return connection;
        }
    }
    return nullptr;
}

void Device::bgpListen() {
    BGPConnection *connection = new BGPConnection(this);

    connection->enqueueEvent(
        BGPEvent::ManualStart_with_PassiveTcpEstablishment);

    bgpConnections.push_back(connection);
}

// ### ReceivedPacketEvent methods

ReceivedPacketEvent::ReceivedPacketEvent(NetworkCard *networkCard,
                                         Description  description)
    : networkCard(networkCard), description(description) {}
