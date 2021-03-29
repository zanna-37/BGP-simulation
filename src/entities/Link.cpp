#include "Link.h"

#include "../logger/Logger.h"

void Link::disconnect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == networkCard) {
        device_source_networkCards.first = nullptr;
    } else if (device_source_networkCards.second == networkCard) {
        device_source_networkCards.second = nullptr;
    } else {
        L_ERROR("This link is not connected to the specified networkCard");
    }
}

void Link::connect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == nullptr) {
        device_source_networkCards.first = networkCard;
    } else if (device_source_networkCards.second == nullptr) {
        device_source_networkCards.second = networkCard;
    } else {
        L_ERROR("This link is already fully connected");
    }
}

NetworkCard *Link::getPeerNetworkCardOrNull(NetworkCard *networkCard) {
    if (device_source_networkCards.first == networkCard) {
        return device_source_networkCards.second;
    } else if (device_source_networkCards.second == networkCard) {
        return device_source_networkCards.first;
    } else {
        return nullptr;
    }
}

void Link::sendPacket(pcpp::Packet *packet, NetworkCard *destination) {
    assert(destination);
    pair<const uint8_t *, int> data = serialize(packet);
    if (connection_status == Connection_status::active) {
        receivePacket(data, destination);
    } else {
        L_ERROR("PHYSICAL LINK BROKEN: " +
                getPeerNetworkCardOrNull(destination)->netInterface + " -> " +
                destination->netInterface);
    }
}

void Link::receivePacket(pair<const uint8_t *, int> data,
                         NetworkCard *              destination) {
    assert(destination);

    uint8_t *rawData = new uint8_t[data.second * sizeof(uint8_t)];

    std::copy(data.first, data.first + data.second * sizeof(uint8_t), rawData);


    pcpp::Packet *receivedPacket =
        deserialize((uint8_t *)rawData, data.second * sizeof(uint8_t));
    destination->receivePacket(receivedPacket);
    delete receivedPacket;
}

pair<const uint8_t *, int> Link::serialize(pcpp::Packet *packet) {
    pcpp::RawPacket *rawPacket = packet->getRawPacket();

    const uint8_t *rawData    = rawPacket->getRawData();
    int            rawDataLen = rawPacket->getRawDataLen();

    // TODO need it?
    // delete packet;

    return std::make_pair(rawData, rawDataLen);
}

pcpp::Packet *Link::deserialize(uint8_t *rawData, int rawDataLen) {
    struct timespec timestamp;
    timespec_get(&timestamp, TIME_UTC);
    pcpp::RawPacket *rawPacket =
        new pcpp::RawPacket(rawData, rawDataLen, timestamp, true);

    pcpp::Packet *packet = new pcpp::Packet(rawPacket, true);

    return packet;
}
