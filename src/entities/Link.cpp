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
    delete packet;
    if (connection_status == Connection_status::active) {
        L_DEBUG("Sending packet through link: " +
                getPeerNetworkCardOrNull(destination)->owner->ID + ":" +
                getPeerNetworkCardOrNull(destination)->netInterface + " -> " +
                destination->owner->ID + ":" + destination->netInterface);
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


    pcpp::Packet *receivedPacket =
        deserialize((uint8_t *)data.first, data.second * sizeof(uint8_t));
    destination->receivePacket(receivedPacket);
}

pair<const uint8_t *, int> Link::serialize(pcpp::Packet *packet) {
    uint8_t *rawData =
        new uint8_t[packet->getRawPacket()->getRawDataLen() * sizeof(uint8_t)];
    std::copy(packet->getRawPacket()->getRawData(),
              packet->getRawPacket()->getRawData() +
                  packet->getRawPacket()->getRawDataLen() * sizeof(uint8_t),
              rawData);
    int rawDataLen = packet->getRawPacket()->getRawDataLen();

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
