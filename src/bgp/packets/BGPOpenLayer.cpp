#include "BGPOpenLayer.h"

void BGPOpenLayer::computeCalculateFieldsInternal() const {
    BGPOpenHeader* openHeader = getOpenHeader();

    openHeader->version = 4;
    openHeader->optionalParametersLength =
        0;  // TODO change me if we support optional parameters
}

BGPOpenLayer::BGPOpenHeader* BGPOpenLayer::getOpenHeader() const {
    uint8_t* dataOpenHeader = m_Data;
    return (BGPOpenHeader*)dataOpenHeader;
}
std::string BGPOpenLayer::toStringInternal() const {
    BGPOpenHeader* openHeader = getOpenHeader();

    std::string output;
    output += "Version: " + std::to_string(openHeader->version) + "\n";
    output += "My Autonomous System: " +
              std::to_string(be16toh(openHeader->myAutonomousSystemNumber)) +
              "\n";
    output +=
        "Hold time: " + std::to_string(be16toh(openHeader->holdTime)) + "\n";
    output += "BGP Identifier: " +
              std::to_string(be32toh(openHeader->BGPIdentifier)) + "\n";

    return output;
}
BGPOpenLayer::BGPOpenLayer(uint16_t                 myAutonomousSystemNumber,
                           uint16_t                 holdTime,
                           const pcpp::IPv4Address& BGPIdentifier)
    : BGPLayer() {
    uint8_t optionalParamsDataLen =
        0;  // TODO change me if we support optional parameters

    const size_t headerLen = sizeof(BGPOpenHeader) + optionalParamsDataLen;
    m_DataLen              = headerLen;
    m_Data                 = new uint8_t[headerLen];
    memset(m_Data, 0, headerLen);

    BGPOpenHeader* openHeader = getOpenHeader();

    openHeader->length                   = htobe16((uint16_t)headerLen);
    openHeader->myAutonomousSystemNumber = htobe16(myAutonomousSystemNumber);
    openHeader->holdTime                 = htobe16(holdTime);
    openHeader->BGPIdentifier            = BGPIdentifier.toInt();
    openHeader->optionalParametersLength = optionalParamsDataLen;
}
