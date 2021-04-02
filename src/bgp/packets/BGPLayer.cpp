#include "BGPLayer.h"

#include <cassert>
#include <climits>
#include <cstring>


BGPLayer* BGPLayer::parseBGPLayerOrNull(uint8_t*      data,
                                        size_t        dataLength,
                                        Layer*        prevLayer,
                                        pcpp::Packet* packet) {
    if (dataLength < sizeof(BGPLayer::BgpCommonHeader)) {
        return nullptr;
    } else {
        auto* header = (BgpCommonHeader*)data;
        switch (header->type) {
            case OPEN:
                // TODO
                // return
                break;
            case UPDATE:
                // TODO
                // return
                break;
            case NOTIFICATION:
                // TODO
                // return
                break;
            case KEEPALIVE:
                // TODO
                // return
                break;
            default:
                return nullptr;
        }
    }
}

BGPLayer::BgpCommonHeader* BGPLayer::getCommonHeader() const {
    return (BgpCommonHeader*)m_Data;
}

size_t BGPLayer::getHeaderLen() const {
    assert(m_DataLen >= sizeof(BgpCommonHeader));

    uint16_t messageLen = be16toh(getCommonHeader()->length);

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
    std::string output = "BGP Message\n";
    output +=
        "Type: " +
        getBGPMessageTypeName((BGPMessageType)getCommonHeader()->type) + "\n" +
        "Lenght: " + std::to_string((BGPMessageType)getCommonHeader()->length);

    output += toStringInternal();

    return output;
};

void BGPLayer::computeCalculateFields() {
    BgpCommonHeader* header = getCommonHeader();

    // every bit of the marker should be filled with ones as per RFC
    memset(header,
           UINT_MAX,
           sizeof(BgpCommonHeader::marker));  // FIXME Check if the header is
                                              // really filled with ones
    header->type   = getBGPMessageType();
    header->length = htobe16(getHeaderLen());
}
