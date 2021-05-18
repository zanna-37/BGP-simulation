#include "Link.h"

#include "../logger/Logger.h"

void Link::disconnect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == networkCard) {
        device_source_networkCards.first = nullptr;
    } else if (device_source_networkCards.second == networkCard) {
        device_source_networkCards.second = nullptr;
    } else {
        L_ERROR("Link", "Not connected to the specified networkCard");
    }
}

void Link::connect(NetworkCard *networkCard) {
    if (device_source_networkCards.first == nullptr) {
        device_source_networkCards.first = networkCard;
    } else if (device_source_networkCards.second == nullptr) {
        device_source_networkCards.second = networkCard;
    } else {
        L_ERROR("Link", "Already fully connected");
    }
}

NetworkCard *Link::getPeerNetworkCardOrNull(
    const NetworkCard *networkCard) const {
    if (device_source_networkCards.first == networkCard) {
        return device_source_networkCards.second;
    } else if (device_source_networkCards.second == networkCard) {
        return device_source_networkCards.first;
    } else {
        return nullptr;
    }
}

void Link::sendPacket(const pcpp::Packet *packet,
                      NetworkCard *       destination) const {
    assert(destination);
    pair<const uint8_t *, int> data = serialize(packet);
    if (connection_status == Connection_status::ACTIVE) {
        L_DEBUG(getPeerNetworkCardOrNull(destination)->owner->ID,
                "Sending packet through link: " +
                    getPeerNetworkCardOrNull(destination)->owner->ID + ":" +
                    getPeerNetworkCardOrNull(destination)->netInterface +
                    " -> " + destination->owner->ID + ":" +
                    destination->netInterface);
        receivePacket(data, destination);
    } else {
        L_ERROR(getPeerNetworkCardOrNull(destination)->owner->ID,
                "PHYSICAL LINK BROKEN: " +
                    getPeerNetworkCardOrNull(destination)->netInterface +
                    " -> " + destination->netInterface);
    }
}

void Link::receivePacket(pair<const uint8_t *, const int> receivedDataStream,
                         NetworkCard *                    destination) {
    assert(destination);

    int      rawDataLen;
    uint8_t *rawData;

    {
        const uint8_t *receivedData    = receivedDataStream.first;
        const int      receivedDataLen = receivedDataStream.second;

        rawDataLen = receivedDataLen;
        rawData    = new uint8_t[rawDataLen];

        std::copy(receivedData, receivedData + receivedDataLen, rawData);
    }

    std::unique_ptr<pcpp::Packet> receivedPacket =
        deserialize(rawData, rawDataLen);
    destination->receivePacket(receivedPacket);
}

pair<const uint8_t *, const int> Link::serialize(const pcpp::Packet *packet) {
    pcpp::RawPacket *rawPacket = packet->getRawPacketReadOnly();
    return std::make_pair(rawPacket->getRawData(), rawPacket->getRawDataLen());
}

unique_ptr<pcpp::Packet> Link::deserialize(uint8_t *rawData, int rawDataLen) {
    struct timespec timestamp;
    timespec_get(&timestamp, TIME_UTC);

    auto *rawPacket = new pcpp::RawPacket(rawData, rawDataLen, timestamp, true);
    auto  packet    = make_unique<pcpp::Packet>(rawPacket, true);

    return packet;
}
