#include "Device.h"


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
    for (NetworkCard *networkCard : *networkCards) {
        delete networkCard;
    }
    delete networkCards;

    delete listenConnection;
    std::map<std::size_t, TCPConnection *>::iterator it;
    // while (it != tcpConnections.end()) {
    //     delete it->second;
    // }

    for (it = tcpConnections.begin(); it != tcpConnections.end(); ++it) {
        delete it->second;
    }
    delete deviceThread;
}

void Device::addCards(vector<NetworkCard *> *networkCards) {
    this->networkCards = networkCards;
}


void Device::start() {
    for (NetworkCard *networkCard : *networkCards) {
        pcpp::IPv4Address networkIP(
            (networkCard->IP.toInt() & networkCard->netmask.toInt()));

        TableRow row(networkIP,
                     networkCard->netmask,
                     pcpp::IPv4Address::Zero,
                     networkCard->netInterface,
                     networkCard);

        routingTable.insertRow(row);

        if (defaultGateway.isValid() &&
            defaultGateway.matchSubnet(networkIP, networkCard->netmask)) {
            TableRow row(pcpp::IPv4Address::Zero,
                         pcpp::IPv4Address::Zero,
                         defaultGateway,
                         networkCard->netInterface,
                         networkCard);

            routingTable.insertRow(row);
        }
    }

    routingTable.printTable();

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

    NetworkCard *nextHopNetworkCard = routingTable.findNextHop(dstAddress);
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
        NetworkCard *nextHopNetworkCard = routingTable.findNextHop(dstAddress);
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
    layers->pop();

    TCPConnection *existingConnection =
        getExistingConnectionOrNull(ipLayer, tcpLayer);
    if (existingConnection != nullptr) {
        // processTCP flags

        L_DEBUG("existing connection");
    } else {
        if (listenConnection != nullptr &&
            listenConnection->srcPort == tcpLayer->getDstPort() &&
            tcpLayer->getTcpHeader()->synFlag) {
            L_DEBUG("SYN Arrived");
            listenConnection->srcAddr = ipLayer->getDstIPv4Address();
            listenConnection->srcPort = tcpLayer->getDstPort();
            listenConnection->dstAddr = ipLayer->getSrcIPv4Address();
            listenConnection->dstPort = tcpLayer->getDstPort();
            addTCPConnection(listenConnection);
            listenConnection->enqueueEvent(ReceiveClientSYN_SendSYNACK);

            // create new listening connection (allocate TCB)
            listenConnection          = new TCPConnection(this);
            listenConnection->srcPort = listenConnection->BGPPort;

            // FIXME
            delete ipLayer;
            delete tcpLayer;

        } else {
            L_INFO("PORT closed or server not listening");
            // send reset to the sender
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
    listenConnection->enqueueEvent(PassiveOpen);
}
void Device::connect(pcpp::IPv4Address *dstAddr, uint16_t dstPort) {
    TCPConnection *connection = new TCPConnection(this);

    // connection->srcAddr = srcAddr;
    // connection->srcPort = srcPort;
    connection->dstAddr = *dstAddr;
    connection->dstPort = dstPort;

    //
    tcpConnections[tcpConnectionHash(*dstAddr, dstPort)] = connection;
    connection->enqueueEvent(ActiveOpen_SendSYN);
    // sendPacket(layers);
}

TCPConnection *Device::getExistingConnectionOrNull(pcpp::IPv4Layer *ipLayer,
                                                   pcpp::TcpLayer * tcpLayer) {
    auto search = tcpConnections.find(tcpConnectionHash(
        ipLayer->getDstIPv4Address(), tcpLayer->getDstPort()));

    if (search != tcpConnections.end()) {
        return search->second;
    }

    return nullptr;
}

void Device::addTCPConnection(TCPConnection *connection) {
    tcpConnections[tcpConnectionHash(connection->dstAddr,
                                     connection->dstPort)] = connection;
}


std::size_t Device::tcpConnectionHash(pcpp::IPv4Address dstAddr,
                                      uint16_t          dstPort) {
    size_t returnHash;

    size_t hash1 = std::hash<std::string>{}(dstAddr.toString());
    size_t hash2 = std::hash<std::string>{}(to_string(dstPort));

    return hash1 ^ hash2;
}
// ReceivedPacketEvent methods

ReceivedPacketEvent::ReceivedPacketEvent(NetworkCard *networkCard,
                                         Description  description)
    : networkCard(networkCard), description(description) {}
