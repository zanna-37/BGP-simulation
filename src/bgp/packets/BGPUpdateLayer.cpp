#include "BGPUpdateLayer.h"

#include <cassert>

void BGPUpdateLayer::computeCalculateFieldsInternal() const {
    // There is no field to calculate in this layer.
}

std::string BGPUpdateLayer::toStringInternal() const {
    std::string output;

    output += "-- Withdrawn routes length: " +
              std::to_string(getWithdrawnRoutesBytesLength()) + "\n";
    std::vector<LengthAndIpPrefix>  withdrawnRoutesList;
     getWithdrawnRoutes(withdrawnRoutesList);
    output +=
        std::to_string(withdrawnRoutesList.size()) + " Withdrawn routes\n";
    for (const auto& withdrawnRoute : withdrawnRoutesList) {
        output += withdrawnRoute.toString();
    }

    output += "-- Path attributes length: " +
              std::to_string(getPathAttributesBytesLength()) + "\n";
    std::vector<PathAttribute>    pathAttributesList ;
    getPathAttributes(pathAttributesList);
    output += std::to_string(pathAttributesList.size()) + " Path attributes\n";
    for (const auto& pathAttributes : pathAttributesList) {
        output += pathAttributes.toString();
    }

    output += "-- Network layer reachability info length: " +
              std::to_string(getNetworkLayerReachabilityInfoBytesLength()) +
              "\n";
    std::vector<LengthAndIpPrefix>   nlriList ;
    getNetworkLayerReachabilityInfo(nlriList);
    output +=
        std::to_string(nlriList.size()) + " Network layer reachability info\n";
    for (const auto& nlri : nlriList) {
        output += nlri.toString();
    }

    return output;
}

BGPUpdateLayer::BGPUpdateLayer(
    const std::vector<LengthAndIpPrefix>& withdrawnRoutes,
    const std::vector<PathAttribute>&     pathAttributes,
    const std::vector<LengthAndIpPrefix>& nlri)
    : BGPLayer() {
    size_t bufferSize =
        4096  // Maximum BGP packet size in octets allowed by RFC
        - sizeof(BGPCommonHeader);

    uint8_t withdrawnRoutesData[bufferSize];
    uint8_t pathAttributesData[bufferSize];
    uint8_t nlriData[bufferSize];

    size_t bufferSizeLeft = bufferSize;

    size_t withdrawnRoutesDataLen =
        LengthAndIpPrefix::lengthAndIpPrefixDataToByteArray(
            withdrawnRoutes, withdrawnRoutesData, bufferSizeLeft);
    bufferSizeLeft -= withdrawnRoutesDataLen;

    size_t pathAttributesDataLen = PathAttribute::pathAttributesToByteArray(
        pathAttributes, pathAttributesData, bufferSizeLeft);
    bufferSizeLeft -= pathAttributesDataLen;

    size_t nlriDataLen = LengthAndIpPrefix::lengthAndIpPrefixDataToByteArray(
        nlri, nlriData, bufferSizeLeft);


    const size_t headerLen =
        sizeof(BGPCommonHeader)   // Space for: Common header Value
        + sizeof(uint16_t)        // Space for: Withdrawn Routes Length
        + withdrawnRoutesDataLen  // Space for: Withdrawn Routes
        + sizeof(uint16_t)        // Space for: Total Path Attribute Length
        + pathAttributesDataLen   // Space for: Path Attributes
        + nlriDataLen;            // Space for: Network Layer Reachability
                                  //            Information Value

    m_DataLen = headerLen;
    m_Data    = new uint8_t[headerLen];
    memset(m_Data, 0, headerLen);

    BGPCommonHeader* commonHeader =
        getCommonHeaderOrNull();  // commonHeader is not null as we just
                                  // populated m_Data and m_DataLen
    commonHeader->length_be = htobe16(headerLen);

    uint8_t* dataPtr = m_Data + sizeof(BGPCommonHeader);

    // COPY WITHDRAWN ROUTES DATA
    // '--> Withdrawn Routes Length
    uint16_t withdrawnRoutesDataLenBE = htobe16(withdrawnRoutesDataLen);
    memcpy(dataPtr, &withdrawnRoutesDataLenBE, sizeof(uint16_t));
    dataPtr += sizeof(uint16_t);

    // '--> Withdrawn Routes
    if (withdrawnRoutesDataLen > 0) {
        memcpy(dataPtr, withdrawnRoutesData, withdrawnRoutesDataLen);
        dataPtr += withdrawnRoutesDataLen;
    }

    // COPY PATH ATTRIBUTES DATA
    // '--> Total Path Attribute Length
    uint16_t pathAttributesDataLenBE = htobe16(pathAttributesDataLen);
    memcpy(dataPtr, &pathAttributesDataLenBE, sizeof(uint16_t));
    dataPtr += sizeof(uint16_t);

    // '--> Path Attributes
    if (pathAttributesDataLen > 0) {
        memcpy(dataPtr, pathAttributesData, pathAttributesDataLen);
        dataPtr += pathAttributesDataLen;
    }

    // COPY NLRI DATA
    // '--> Network Layer Reachability Information Value
    if (nlriDataLen > 0) {
        memcpy(dataPtr, nlriData, nlriDataLen);
    }
}

size_t BGPUpdateLayer::getWithdrawnRoutesBytesLength() const {
    size_t headerLen = getHeaderLen();
    size_t minLen = sizeof(BGPCommonHeader)  // Space for: Common header Value
                    + sizeof(uint16_t);  // Space for: Withdrawn Routes Length

    if (headerLen < minLen) {
        return 0;
    } else {
        uint16_t res = be16toh(*(uint16_t*)(m_Data + sizeof(BGPCommonHeader)));
        if ((size_t)res > headerLen - minLen) {
            return headerLen - minLen;
        }

        return (size_t)res;
    }
}

void BGPUpdateLayer::getWithdrawnRoutes(
    std::vector<LengthAndIpPrefix>& withdrawnRoutes) const {

    size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();
    if (withdrawnRouteLen != 0) {
        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t);      // Space for: Withdrawn Routes Length
        uint8_t* dataPtr = m_Data + offset;

        LengthAndIpPrefix::parsePrefixAndIPData(
            dataPtr, withdrawnRouteLen, withdrawnRoutes);
    }
}

size_t BGPUpdateLayer::getPathAttributesBytesLength() const {
    size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();

    size_t headerLen = getHeaderLen();
    size_t minLen = sizeof(BGPCommonHeader)  // Space for: Common header Value
                    + sizeof(uint16_t)   // Space for: Withdrawn Routes Length
                    + withdrawnRouteLen  // Space for: Withdrawn Routes
                    +
                    sizeof(uint16_t);  // Space for: Total Path Attribute Length

    if (headerLen >= minLen) {
        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t)       // Space for: Withdrawn Routes Length
            + withdrawnRouteLen;     // Space for: Withdrawn Routes

        uint16_t res = be16toh(*(uint16_t*)(m_Data + offset));
        if ((size_t)res > headerLen - offset) {
            return headerLen - offset;
        }

        return (size_t)res;
    }

    return 0;
}

void BGPUpdateLayer::getPathAttributes(
    std::vector<PathAttribute>& pathAttributes) const {

    size_t pathAttrLen = getPathAttributesBytesLength();
    if (pathAttrLen != 0) {
        size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();
        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t)       // Space for: Withdrawn Routes Length
            + withdrawnRouteLen      // Space for: Withdrawn Routes
            + sizeof(uint16_t);      // Space for: Total Path Attribute Length

        assert(m_DataLen >= offset + pathAttrLen);
        uint8_t* dataPtr = m_Data + offset;

        PathAttribute::parsePathAttributes(
            dataPtr, pathAttrLen, pathAttributes);
    }
}

size_t BGPUpdateLayer::getNetworkLayerReachabilityInfoBytesLength() const {
    size_t headerLen         = getHeaderLen();
    size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();
    size_t pathAttrLen       = getPathAttributesBytesLength();

    size_t minLen = sizeof(BGPCommonHeader)  // Space for: Common header Value
                    + sizeof(uint16_t)   // Space for: Withdrawn Routes Length
                    + withdrawnRouteLen  // Space for: Withdrawn Routes
                    +
                    sizeof(uint16_t)  // Space for: Total Path Attribute Length
                    + pathAttrLen;    // Space for: Path Attributes

    if (headerLen >= minLen) {
        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t)       // Space for: Withdrawn Routes Length
            + withdrawnRouteLen      // Space for: Withdrawn Routes
            + sizeof(uint16_t)       // Space for: Total Path Attribute Length
            + pathAttrLen;           // Space for: Path Attributes

        size_t nlriSize = headerLen - offset;

        assert(((int)headerLen - (int)offset) >= 0);

        return nlriSize;
    }

    return 0;
}

void BGPUpdateLayer::getNetworkLayerReachabilityInfo(
    std::vector<LengthAndIpPrefix>& nlri) const {

    size_t nlriSize = getNetworkLayerReachabilityInfoBytesLength();
    if (nlriSize != 0) {
        size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();
        size_t pathAttrLen       = getPathAttributesBytesLength();

        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t)       // Space for: Withdrawn Routes Length
            + withdrawnRouteLen      // Space for: Withdrawn Routes
            + sizeof(uint16_t)       // Space for: Total Path Attribute Length
            + pathAttrLen;           // Space for: Path Attributes

        uint8_t* dataPtr = m_Data + offset;

        LengthAndIpPrefix::parsePrefixAndIPData(dataPtr, nlriSize, nlri);
    }
}