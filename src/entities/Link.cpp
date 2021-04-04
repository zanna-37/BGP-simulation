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

void Link::sendPacketThroughWire(std::pair<const uint8_t *, int> data,
                                 NetworkCard *destination) const {
    assert(destination);

    if (connection_status == Connection_status::ACTIVE) {
        L_VERBOSE("link",
                  getLogLinkName(destination) + ": sending data through link");
        destination->receivePacketFromWire(data);
    } else {
        L_ERROR("link", getLogLinkName(destination) + ": physical link broken");
    }
}

std::string Link::getLogLinkName(NetworkCard *destination) const {
    NetworkCard *source = getPeerNetworkCardOrNull(destination);

    std::string output;

    output += source ? source->owner->ID + " " + source->netInterface : "null";
    output += source && destination ? " -> " : " <-?-> ";
    output += destination
                  ? destination->owner->ID + " " + destination->netInterface
                  : "null";

    return output;
}

string Link::getConnecionStatusString() {
    switch (connection_status) {
        case ACTIVE:
            return "active";
            break;

        case FAILED:
            return "failed";
            break;

        default:
            L_ERROR("Link", "UNKNOWN connection status"); //TODO put link name
            return "UNKNOWN";
            break;
    }
}