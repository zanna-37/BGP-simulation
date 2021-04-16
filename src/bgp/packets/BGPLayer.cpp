#include "BGPLayer.h"

#include <Logger.h>

#include <climits>
#include <cstring>

#include "BGPOpenLayer.h"


BGPLayer* BGPLayer::parseBGPLayerOrNull(uint8_t*      data,
                                        size_t        dataLength,
                                        Layer*        prevLayer,
                                        pcpp::Packet* packet) {
    if (dataLength < sizeof(BGPLayer::BGPCommonHeader)) {
        return nullptr;
    } else {
        auto* header = (BGPCommonHeader*)data;
        switch (header->type) {
            case OPEN:
                return new BGPOpenLayer(data, dataLength, prevLayer, packet);
            case UPDATE:
                // TODO
                // return
            case NOTIFICATION:
                // TODO
                // return
            case KEEPALIVE:
                // TODO
                // return
            default:
                return nullptr;
        }
    }
}

BGPLayer::BGPCommonHeader* BGPLayer::getCommonHeaderOrNull() const {
    if (m_DataLen >= sizeof(BGPCommonHeader))
        return (BGPCommonHeader*)m_Data;
    else {
        LOG_ERROR("The packet is to small to fit the BGP common header");
        return nullptr;
    }
}

size_t BGPLayer::getHeaderLen() const {
    uint16_t messageLen = sizeof(BGPCommonHeader) + getHeaderLenInternal();

    // The following check is needed because, when parsing a received packet, we
    // must take into account that the field `length` might be wrong or invalid.
    // Or maybe the packet is longer than the declared length and it is filled
    // with garbage.
    if (m_DataLen < messageLen) {
        return m_DataLen;
    } else {
        return messageLen;
    }
}

std::string BGPLayer::getBGPMessageTypeName(BGPLayer::BGPMessageType type) {
    switch (type) {
        case BGPLayer::OPEN:
            return "OPEN";
        case BGPLayer::UPDATE:
            return "UPDATE";
        case BGPLayer::NOTIFICATION:
            return "NOTIFICATION";
        case BGPLayer::KEEPALIVE:
            return "KEEPALIVE";
        default:
            return "UNKNOWN TYPE";
    }
}

std::string BGPLayer::toString() const {
    BGPCommonHeader* commonHeader = getCommonHeaderOrNull();
    if (commonHeader) {
        std::string output = "BGP Message\n";
        output += "Type: " +
                  getBGPMessageTypeName((BGPMessageType)commonHeader->type) +
                  "\n";
        output +=
            "Lenght: " + std::to_string(be16toh(commonHeader->length_be)) +
            "\n";

        output += "-- Message header --\n";
        output += toStringInternal();

        return output;
    } else {
        return "";
    }
};

void BGPLayer::computeCalculateFields() {
    BGPCommonHeader* header = getCommonHeaderOrNull();
    if (header) {
        // every bit of the marker should be filled with ones as per RFC
        memset(header,
               UINT_MAX,
               sizeof(BGPCommonHeader::marker));  // FIXME Check if the header
                                                  // is really filled with ones
        header->length_be = htobe16((uint16_t)getHeaderLen());
        header->type      = getBGPMessageType();

        computeCalculateFieldsInternal();
    }
}
