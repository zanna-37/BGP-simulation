#include "NetworkCard.h"

#include "../logger/Logger.h"

NetworkCard::NetworkCard(std::string       netInterface,
                         pcpp::IPv4Address IP,
                         pcpp::IPv4Address netmask,
                         pcpp::MacAddress  mac,
                         Device*           owner)
    : netInterface(std::move(netInterface)),
      IP(IP),
      netmask(netmask),
      owner(owner) {
    if (mac != pcpp::MacAddress::Zero) {
        this->mac = mac;
    } else {
        macGenerator(this->mac);
    }
}

NetworkCard::~NetworkCard() = default;

void NetworkCard::connect(const std::shared_ptr<Link>& linkToConnect) {
    if (link == nullptr) {
        linkToConnect->connect(this);
        link = linkToConnect;
    } else {
        L_ERROR(owner->ID,
                "(" + netInterface + ") This interface is already connected");
    }
}

void NetworkCard::disconnect(const std::shared_ptr<Link>& linkToDisconnect) {
    if (link.get() == linkToDisconnect.get()) {
        linkToDisconnect->disconnect(this);
        link = nullptr;
    } else if (link == nullptr) {
        L_ERROR(owner->ID,
                "(" + netInterface + ") This interface has no link connected");
    } else {
        L_ERROR(
            owner->ID,
            "(" + netInterface +
                ") This interface is not connected with the link specified");
    }
}

void NetworkCard::sendPacket(
    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers) {
    NetworkCard* destination = link->getPeerNetworkCardOrNull(this);

    if (destination == nullptr) {
        L_ERROR(owner->ID,
                "No destination for packets from from " + netInterface);
    } else {
        L_DEBUG(owner->ID,
                "Sending packet from " + netInterface + " through link");

        auto* packet = new pcpp::Packet(100);

        packet->addLayer(new pcpp::EthLayer(mac, destination->mac), true);
        while (!layers->empty()) {
            auto layer = std::move(layers->top());
            layers->pop();

            packet->addLayer(layer.release(), true);
        }
        packet->computeCalculateFields();

        std::pair<const uint8_t*, const int> data = serialize(packet);
        link->sendPacketThroughWire(data, destination);
        delete packet;
    }
}

void NetworkCard::receivePacketFromWire(
    std::pair<const uint8_t*, const int> receivedDataStream) {
    int      rawDataLen;
    uint8_t* rawData;

    {
        // Create a copy of the received stream
        const uint8_t* receivedData    = receivedDataStream.first;
        const int      receivedDataLen = receivedDataStream.second;

        rawDataLen = receivedDataLen;
        rawData    = new uint8_t[rawDataLen];

        std::copy(receivedData, receivedData + receivedDataLen, rawData);
        receivedDataStream
            .~pair();  // Ensure the old object is not used further
    }

    std::unique_ptr<pcpp::Packet> receivedPacket =
        NetworkCard::deserialize(rawData, rawDataLen);

    L_DEBUG(owner->ID, "Enqueueing packet in " + netInterface + " queue");

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers =
        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();

    pcpp::Layer* currentLayer = receivedPacket->getLastLayer();
    while (currentLayer != nullptr) {
        pcpp::ProtocolType protocol = currentLayer->getProtocol();
        layers->push(std::move(std::unique_ptr<pcpp::Layer>(
            receivedPacket->detachLayer(protocol))));
        currentLayer = receivedPacket->getLastLayer();
    }

    // Extract, process, and dispose the ethernet layer
    auto layer = std::move(layers->top());
    layers->pop();
    auto* ethLayer_weak = dynamic_cast<pcpp::EthLayer*>(layer.get());
    // TODO compute mac layer stuff
    bool isPacketForUs =
        true;                 // TODO change and use the MAC address to find out
    ethLayer_weak = nullptr;  // Ensure it is no more used
    layer.reset();

    if (isPacketForUs) {
        std::unique_lock<std::mutex> receivedPacketsQueue_lock(
            receivedPacketsQueue_mutex);
        receivedPacketsQueue.push(std::move(layers));
        receivedPacketsQueue_wakeup.notify_one();
        receivedPacketsQueue_lock.unlock();
    } else {
        L_ERROR(owner->ID, netInterface + ": Discarding packet not for us");
    }
}

std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
NetworkCard::waitForL3Packet() {
    std::unique_lock<std::mutex> receivedPacketsQueue_lock(
        receivedPacketsQueue_mutex);

    while (receivedPacketsQueue.empty() && running) {
        receivedPacketsQueue_wakeup.wait(receivedPacketsQueue_lock);
    }

    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>> layers;

    if (running) {
        layers = std::move(receivedPacketsQueue.front());
        receivedPacketsQueue.pop();
    } else {
        receivedPacketsQueue_lock.unlock();
    }

    return std::move(layers);
}


std::pair<const uint8_t*, const int> NetworkCard::serialize(
    const pcpp::Packet* packet) {
    pcpp::RawPacket* rawPacket = packet->getRawPacketReadOnly();
    return std::make_pair(rawPacket->getRawData(), rawPacket->getRawDataLen());
}

std::unique_ptr<pcpp::Packet> NetworkCard::deserialize(uint8_t*  rawData,
                                                       const int rawDataLen) {
    struct timespec timestamp;
    timespec_get(&timestamp,
                 TIME_UTC);  // TODO do we have to use local time here maybe?

    auto* rawPacket = new pcpp::RawPacket(rawData, rawDataLen, timestamp, true);
    auto  packet    = std::make_unique<pcpp::Packet>(rawPacket, true);

    return packet;
}

void NetworkCard::shutdown() {
    running = false;
    receivedPacketsQueue_wakeup.notify_all();
}
