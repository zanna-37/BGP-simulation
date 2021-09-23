#include "BGPOpenLayer.h"

#include <Logger.h>
#include <endian.h>
#include <string.h>

#include "../../logger/Logger.h"


BGPOpenLayer::BGPOpenHeader* BGPOpenLayer::getOpenHeaderOrNull() const {
    if (m_DataLen >= sizeof(BGPOpenHeader))
        return (BGPOpenHeader*)m_Data;
    else {
        LOG_ERROR("The packet is to small to fit the BGP open header");
        return nullptr;
    }
}

void BGPOpenLayer::computeCalculateFieldsInternal() const {
    BGPOpenHeader* openHeader = getOpenHeaderOrNull();
    if (openHeader) {
        openHeader->version = BGPOpenLayer::version;
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

    const size_t headerLen = sizeof(BGPOpenHeader) + optionalParamsDataLen;
    m_DataLen              = headerLen;
    m_Data                 = new uint8_t[headerLen];
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

bool BGPOpenLayer::checkMessageErr(uint8_t subcode) const {
    // OPEN Message Error subcodes:

    //      1 - Unsupported Version Number.
    //      2 - Bad Peer AS.
    //      3 - Bad BGP Identifier.
    //      4 - Unsupported Optional Parameter.
    //      5 - [Deprecated - see Appendix A].
    //      6 - Unacceptable Hold Time.

    BGPOpenHeader* openHeader = getOpenHeaderOrNull();
    if (openHeader->version != BGPOpenLayer::version) {
        subcode = 1;
        L_ERROR("OpenMSGErr", "BGP version not supported");
        return false;
    } else if (!checkAS()) {
        subcode = 2;
        L_ERROR("OpenMSGErr", "Peer AS number invalid");
        return false;
    } else if (be16toh(openHeader->holdTime_be) == 1 || be16toh(openHeader->holdTime_be) == 2) {
        subcode = 6;
        L_ERROR("OpenMSGErr", "Unacceptable Hold Time");
        return false;
    } else if (!checkValidIP()) {
        subcode = 3;
        L_ERROR("OpenMSGErr", "BGP Identifier is not an unicast IP address");
        return false;
    } else if (openHeader->optionalParametersLength > 0) {
        subcode = 4;
        L_ERROR("OpenMSGErr", "No optional paramenter are supported");
        return false;
    }

    return true;
}

bool BGPOpenLayer::checkValidIP() const {
    BGPOpenHeader* openHeader = getOpenHeaderOrNull();

    if (openHeader->BGPIdentifier_be == 0x00000000) {
        L_ERROR("Verify IP", "The BGP Identifier is invalid -> 0.0.0.0");
        return false;
    } else if (openHeader->BGPIdentifier_be >= 0xE0000000 &&
               openHeader->BGPIdentifier_be <= 0xEFFFFFFF) {
        L_ERROR("Verify IP", "The BGP Identifier is a multicast IP");
        return false;
    } else if ((openHeader->BGPIdentifier_be >= 0x0A000000 &&
                openHeader->BGPIdentifier_be <= 0x0AFFFFFF) ||
               (openHeader->BGPIdentifier_be >= 0xAC100000 &&
                openHeader->BGPIdentifier_be <= 0xAC10FFFF) ||
               (openHeader->BGPIdentifier_be >= 0xC0A80000 &&
                openHeader->BGPIdentifier_be <= 0xC0A800FF)) {
        L_ERROR("Verify IP", "The BGP Identifier is a private IP");
        return false;
    }
    return true;
}