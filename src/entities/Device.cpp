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

void Device::sendPacket(stack<pcpp::Layer *> *layers,
                        NetworkCard *         networkCard) {
    L_DEBUG("Sending packet from " + ID + " using " +
            networkCard->netInterface);
    networkCard->sendPacket(layers);
}

void Device::receivePacket(stack<pcpp::Layer *> *layers, NetworkCard *origin) {
    pcpp::IPv4Layer * ipLayer = dynamic_cast<pcpp::IPv4Layer *>(layers->top());
    pcpp::IPv4Address dstAddress = ipLayer->getDstIPv4Address();

    if (dstAddress == origin->IP) {
        L_DEBUG("processing message");
        processMessage(layers);
    } else {
        TableRow *nextHop_row = routingTable.findNextHop(dstAddress);
        if (nextHop_row == nullptr) {
            L_ERROR("DESTINATION UNREACHABLE");
        } else {
            forwardMessage(layers, nextHop_row->networkCard);
        }
    }
}

void Device::processMessage(stack<pcpp::Layer *> *layers) {
    L_DEBUG("RICEVUTO: " + ID);

    // TODO pass layer to TCP
    while (!(layers->empty())) {
        pcpp::Layer *layer = layers->top();
        layers->pop();
        delete layer;
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

// ReceivedPacketEvent methods

ReceivedPacketEvent::ReceivedPacketEvent(NetworkCard *networkCard,
                                         Description  description)
    : networkCard(networkCard), description(description) {}
