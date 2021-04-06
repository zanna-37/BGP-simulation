#include "Device.h"

#include <iomanip>
#include <iostream>

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

    unique_lock<std::mutex> packetsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);
    receivedPacketsEventQueue_wakeup.notify_one();
    packetsEventQueue_uniqueLock.unlock();
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
        pcpp::IPv4Address *networkIP = new pcpp::IPv4Address(
            (networkCard->IP.toInt() & networkCard->netmask.toInt()));

        TableRow *row =
            new TableRow(networkIP,
                         &(networkCard->netmask),
                         new pcpp::IPv4Address(pcpp::IPv4Address::Zero),
                         networkCard->netInterface,
                         networkCard);

        routingTable->push_back(row);

        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(*networkIP, networkCard->netmask)) {
            TableRow *row =
                new TableRow(new pcpp::IPv4Address(pcpp::IPv4Address::Zero),
                             new pcpp::IPv4Address(pcpp::IPv4Address::Zero),
                             &defaultGateway,
                             networkCard->netInterface,
                             networkCard);

            routingTable->push_back(row);
        }
    }

    printTable();

    running      = true;
    deviceThread = new std::thread([&]() {
        while (running) {
            unique_lock<std::mutex> packetsEventQueue_uniqueLock(
                receivedPacketsEventQueue_mutex);
            while (receivedPacketsEventQueue.empty() && running) {
                receivedPacketsEventQueue_wakeup.wait(
                    packetsEventQueue_uniqueLock);

                if (receivedPacketsEventQueue.empty() && running) {
                    L_DEBUG("Spurious Wakeup");
                }
            }

            if (running) {
                L_DEBUG("Queue not empty: handling event");
                ReceivedPacketEvent *event = receivedPacketsEventQueue.front();
                L_DEBUG("Packet arrived at " +
                        event->networkCard->netInterface);
                receivedPacketsEventQueue.pop();

                event->networkCard->handleNextPacket();

                delete event;
            } else {
                L_VERBOSE("Shutting down:" + ID);
            }
            packetsEventQueue_uniqueLock.unlock();
        }
    });
}

void Device::sendPacket(stack<pcpp::Layer *> *layers) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    NetworkCard *nextHopNetworkCard = findNextHop(&dstAddress);
    if (nextHopNetworkCard == nullptr) {
        L_ERROR("DESTINATION UNREACHABLE");
    } else {
        L_DEBUG("Sending packet from " + ID + " using " +
                nextHopNetworkCard->netInterface);
        ipLayer->setSrcIPv4Address(nextHopNetworkCard->IP);
        nextHopNetworkCard->sendPacket(layers);
    }
}

void Device::receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    if (dstAddress == origin->IP) {
        L_DEBUG("processing message");
        processMessage(layers);
    } else {
        NetworkCard *nextHopNetworkCard = findNextHop(&dstAddress);
        if (nextHopNetworkCard == nullptr) {
            L_ERROR("DESTINATION UNREACHABLE");
        } else {
            forwardMessage(layers, nextHopNetworkCard);
        }
    }
}

void Device::processMessage(stack<pcpp::Layer *> *layers) {
    L_DEBUG("RICEVUTO: " + ID);

    pcpp::IPv4Layer *ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    layers->pop();
    pcpp::TcpLayer *tcpLayer = dynamic_cast<pcpp::TcpLayer *>(layers->top());
    layers->push(ipLayer);

    TCPConnection *existingConnection =
        getExistingConnectionOrNull(ipLayer, tcpLayer);
    if (existingConnection != nullptr) {
        // processTCP flags

        L_DEBUG("existing connection");
        existingConnection->processMessage(layers);
    } else {
        if (listenConnection != nullptr &&
            listenConnection->srcPort == tcpLayer->getDstPort()) {
            L_DEBUG("Handling new TCP Connection");
            listenConnection->processMessage(layers);
        } else {
            L_INFO("PORT closed or server not listening");
            // TODO send reset to the sender
        }
    }
}

void Device::enqueueEvent(ReceivedPacketEvent *event) {
    L_DEBUG("Enqueueing event in " + ID + " event queue");
    unique_lock<std::mutex> packetsEventQueue_uniqueLock(
        receivedPacketsEventQueue_mutex);

    receivedPacketsEventQueue.push(event);
    receivedPacketsEventQueue_wakeup.notify_one();

    packetsEventQueue_uniqueLock.unlock();
}

void Device::listen() {
    // Initialize passive Open Listen for TCP
    listenConnection          = new TCPConnection(this);
    listenConnection->srcPort = listenConnection->BGPPort;
    listenConnection->enqueueEvent(TCPEvent::PassiveOpen);
}
void Device::connect(pcpp::IPv4Address *dstAddr, uint16_t dstPort) {
    TCPConnection *connection = new TCPConnection(this);

    connection->dstAddr  = *dstAddr;
    connection->dstPort  = dstPort;
    size_t hash          = tcpConnectionHash(dstAddr->toString(), dstPort);
    tcpConnections[hash] = connection;
    connection->enqueueEvent(TCPEvent::ActiveOpen_SendSYN);
}

TCPConnection *Device::getExistingConnectionOrNull(pcpp::IPv4Layer *ipLayer,
                                                   pcpp::TcpLayer * tcpLayer) {
    auto search = tcpConnections.find(tcpConnectionHash(
        ipLayer->getSrcIPv4Address().toString(), tcpLayer->getSrcPort()));

    if (search != tcpConnections.end()) {
        return search->second;
    }

    return nullptr;
}

void Device::addTCPConnection(TCPConnection *connection) {
    tcpConnections[tcpConnectionHash(connection->dstAddr.toString(),
                                     connection->dstPort)] = connection;
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
        if (dstAddress->matchSubnet(*(row->networkIP), *(row->netmask)) &&
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
        printElement(row->networkIP->toString());
        printElement(row->defaultGateway->toString());
        printElement(row->netmask->toString());
        printElement(row->netInterface);
        cout << std::endl;
    }
    std::cout << std::endl;
}

// ### ReceivedPacketEvent methods

ReceivedPacketEvent::ReceivedPacketEvent(NetworkCard *networkCard,
                                         Description  description)
    : networkCard(networkCard), description(description) {}
