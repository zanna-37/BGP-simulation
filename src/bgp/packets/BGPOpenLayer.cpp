#include "BGPOpenLayer.h"

#include <Logger.h>

BGPOpenLayer::BGPOpenHeader* BGPOpenLayer::getOpenHeaderOrNull() const {
    if (m_DataLen >= sizeof(BGPCommonHeader) + sizeof(BGPOpenHeader))
        return (BGPOpenHeader*)m_Data;
    else {
        LOG_ERROR("The packet is to small to fit the BGP open header");
        return nullptr;
    }
}

void BGPOpenLayer::computeCalculateFieldsInternal() const {
    BGPOpenHeader* openHeader = getOpenHeaderOrNull();
    if (openHeader) {
        openHeader->version = 4;
    }
}

std::string BGPOpenLayer::toStringInternal() const {
    BGPOpenHeader* openHeader = getOpenHeaderOrNull();
    if (openHeader) {
        std::string output;
        output += "Version: " + std::to_string(openHeader->version) + "\n";
        output +=
            "My Autonomous System: " +
            std::to_string(be16toh(openHeader->myAutonomousSystemNumber_be)) +
            "\n";
        output +=
            "Hold time: " + std::to_string(be16toh(openHeader->holdTime_be)) +
            "\n";
        output += "BGP Identifier: " +
                  std::to_string(be32toh(openHeader->BGPIdentifier_be)) + " (" +
                  pcpp::IPv4Address(openHeader->BGPIdentifier_be).toString() +
                  ")\n";

        return output;
    } else {
        return "";
    }
}
BGPOpenLayer::BGPOpenLayer(uint16_t                 myAutonomousSystemNumber,
                           uint16_t                 holdTime,
                           const pcpp::IPv4Address& BGPIdentifier)
    : BGPLayer() {
    uint8_t optionalParamsDataLen =
        0;  // TODO change me if we support optional parameters

    const size_t headerLen =
        sizeof(BGPCommonHeader) + sizeof(BGPOpenHeader) + optionalParamsDataLen;
    m_DataLen = headerLen;
    m_Data    = new uint8_t[headerLen];
    memset(m_Data, 0, headerLen);

    BGPOpenHeader* openHeader =
        getOpenHeaderOrNull();  // openHeader is not null as we just populated
                                // m_Data and m_DataLen

    openHeader->length_be                   = htobe16(headerLen);
    openHeader->myAutonomousSystemNumber_be = htobe16(myAutonomousSystemNumber);
    openHeader->holdTime_be                 = htobe16(holdTime);
    openHeader->BGPIdentifier_be            = BGPIdentifier.toInt();
    openHeader->optionalParametersLength    = optionalParamsDataLen;
}
