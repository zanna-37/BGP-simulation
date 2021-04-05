#include "BGPLayer.h"

#include <cassert>
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

BGPLayer::BGPCommonHeader* BGPLayer::getCommonHeader() const {
    return (BGPCommonHeader*)m_Data;
}

size_t BGPLayer::getHeaderLen() const {
    assert(m_DataLen >= sizeof(BGPCommonHeader));

    uint16_t messageLen = be16toh(getCommonHeader()->length);

    // The following check is needed because, when parsing a received packet, we
    // must take into account that the field `length` might be wrong or invalid.
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
    BGPCommonHeader* commonHeader = getCommonHeader();

    std::string output = "BGP Message\n";
    output +=
        "Type: " + getBGPMessageTypeName((BGPMessageType)commonHeader->type) +
        "\n";
    output += "Lenght: " + std::to_string(be16toh(commonHeader->length)) + "\n";

    output += "-- Message header --\n";
    output += toStringInternal();

    return output;
};

void BGPLayer::computeCalculateFields() {
    BGPCommonHeader* header = getCommonHeader();

    // every bit of the marker should be filled with ones as per RFC
    memset(header,
           UINT_MAX,
           sizeof(BGPCommonHeader::marker));  // FIXME Check if the header is
                                              // really filled with ones
    header->type   = getBGPMessageType();
    header->length = htobe16((uint16_t)getHeaderLen());

    computeCalculateFieldsInternal();
}
