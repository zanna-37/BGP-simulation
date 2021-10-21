#include "BGPUpdateLayer.h"

#include <endian.h>
#include <string.h>

#include <cassert>
#include <vector>

#include "../../logger/Logger.h"


void BGPUpdateLayer::computeCalculateFieldsInternal() const {
    // There is no field to calculate in this layer.
}

std::string BGPUpdateLayer::toStringInternal() const {
    std::string output;

    output += "-- Withdrawn routes length: " +
              std::to_string(getWithdrawnRoutesBytesLength()) + "\n";
    std::vector<LengthAndIpPrefix> withdrawnRoutesList;
    getWithdrawnRoutes(withdrawnRoutesList);
    output +=
        std::to_string(withdrawnRoutesList.size()) + " Withdrawn routes\n";
    for (const auto& withdrawnRoute : withdrawnRoutesList) {
        output += withdrawnRoute.toString();
    }

    output += "-- Path attributes length: " +
              std::to_string(getPathAttributesBytesLength()) + "\n";
    std::vector<PathAttribute> pathAttributesList;
    getPathAttributes(pathAttributesList);
    output += std::to_string(pathAttributesList.size()) + " Path attributes\n";
    for (const auto& pathAttributes : pathAttributesList) {
        output += pathAttributes.toString();
    }

    output += "-- Network layer reachability info length: " +
              std::to_string(getNetworkLayerReachabilityInfoBytesLength()) +
              "\n";
    std::vector<LengthAndIpPrefix> nlriList;
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

bool BGPUpdateLayer::checkMessageErr(uint8_t*              subcode,
                                     std::vector<uint8_t>* data_be8) const {
    // UPDATE Message Error subcodes:

    //          1 - Malformed Attribute List.
    //          2 - Unrecognized Well-known Attribute.
    //          3 - Missing Well-known Attribute.
    //          4 - Attribute Flags Error.
    //          5 - Attribute Length Error.
    //          6 - Invalid ORIGIN Attribute.
    //          7 - [Deprecated - see Appendix A].
    //          8 - Invalid NEXT_HOP Attribute.
    //          9 - Optional Attribute Error.
    //         10 - Invalid Network Field.
    //         11 - Malformed AS_PATH.

    size_t wrLength  = getWithdrawnRoutesBytesLength();
    size_t tpaLength = getPathAttributesBytesLength();

    BGPCommonHeader* updateHeader = getCommonHeaderOrNull();

    if (be16toh(updateHeader->length_be) < wrLength + tpaLength + 23) {
        *subcode = 1;
        L_ERROR("UpdatMSGErr",
                "fields length bigger than total length -> Malformed Attribute "
                "List");
        return false;
    } else if (getPathAttributesBytesLength() != 0) {
        std::vector<PathAttribute> pathAttributes;

        getPathAttributes(pathAttributes);

        // Attributes order as in RFC:

        // attribute           EBGP                    IBGP
        //  ORIGIN             mandatory               mandatory
        //  AS_PATH            mandatory               mandatory
        //  NEXT_HOP           mandatory               mandatory
        //  MULTI_EXIT_DISC    discretionary           discretionary
        //  LOCAL_PREF         see Section 5.1.5       required
        //  ATOMIC_AGGREGATE   see Section 5.1.6 and 9.1.4
        //  AGGREGATOR         discretionary           discretionary


        bool checkDoubleAtt[7] = {
            false, false, false, false, false, false, false};

        for (auto pathAtt : pathAttributes) {
            switch (pathAtt.attributeTypeCode) {
                case PathAttribute::AttributeTypeCode_uint8_t::ORIGIN:
                    if (!checkDoubleAtt[0]) {
                        checkDoubleAtt[0] = true;

                        if (pathAtt.getAttributeLength_h() != 1) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }

                        uint8_t* origin_val = pathAtt.getAttributeValue_be();

                        if (*origin_val < 0 || *origin_val > 2) {
                            *subcode = 6;
                            L_ERROR("UpdatMSGErr",
                                    "The ORIGIN attribute has a wrong value");
                            pathAtt.getAttribute_be8(data_be8);
                            return false;
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The ORIGIN attribute is present twice in the "
                                "message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::AS_PATH:
                    if (!checkDoubleAtt[1]) {
                        checkDoubleAtt[1] = true;

                        if (pathAtt.getAttributeLength_h() < 4) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }

                        // XXX: Optional check
                        // If the UPDATE message is received from an external
                        // peer, the local system MAY check whether the leftmost
                        // (with respect to the position of octets in the
                        // protocol message) AS in the AS_PATH attribute is
                        // equal to the autonomous system number of the peer
                        // that sent the message.  If the check determines this
                        // is not the case, the Error Subcode MUST be set to
                        // Malformed AS_PATH.

                        if (!pathAtt.checkAsPathAttribute()) {
                            *subcode = 11;
                            L_ERROR("UpdatMSGErr",
                                    "The AS_PATH attribute is syntactically "
                                    "incorrect");
                            return false;
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The AS_PATH attribute is present twice in the "
                                "message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP:
                    if (!checkDoubleAtt[2]) {
                        checkDoubleAtt[2] = true;

                        if (pathAtt.getAttributeLength_h() != 4) {
                            *subcode = 8;
                            L_ERROR("UpdatMSGErr",
                                    "The NEXT_HOP attribute has wrong length");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        } else {
                            uint8_t* nextHopVal =
                                pathAtt.getAttributeValue_be();
                            uint32_t ipToCheck =
                                nextHopVal[0] | (nextHopVal[1] << 8) |
                                (nextHopVal[2] << 16) | (nextHopVal[3] << 24);
                            if (!BGPUpdateLayer::checkValidIP(ipToCheck)) {
                                *subcode = 8;
                                L_ERROR(
                                    "UpdatMSGErr",
                                    "The NEXT_HOP attribute has Invalid IP");
                                pathAtt.getAttribute_be8(data_be8);

                                return false;
                            }
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The NEXT_HOP attribute is present twice in "
                                "the message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::MULTI_EXIT_DISC:
                    if (!checkDoubleAtt[3]) {
                        checkDoubleAtt[3] = true;

                        if (pathAtt.getAttributeLength_h() != 4) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }


                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The MULTI_EXIT_DISC attribute is present "
                                "twice in the message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::LOCAL_PREF:
                    if (!checkDoubleAtt[4]) {
                        checkDoubleAtt[4] = true;

                        if (pathAtt.getAttributeLength_h() != 4) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The LOCAL_PREF attribute is present twice in "
                                "the message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::ATOMIC_AGGREGATE:
                    if (!checkDoubleAtt[5]) {
                        checkDoubleAtt[5] = true;

                        if (pathAtt.getAttributeLength_h() != 0) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The ATOMIC_AGGREGATE attribute is present "
                                "twice in the message");
                        return false;
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::AGGREGATOR:
                    if (!checkDoubleAtt[6]) {
                        checkDoubleAtt[6] = true;

                        if (pathAtt.getAttributeLength_h() != 6) {
                            *subcode = 5;
                            L_ERROR("UpdatMSGErr",
                                    "The attribute has wrong length given its "
                                    "type");
                            pathAtt.getAttribute_be8(data_be8);

                            return false;
                        }

                    } else {
                        *subcode = 1;
                        L_ERROR("UpdatMSGErr",
                                "The AGGREGATOR attribute is present twice in "
                                "the message");
                        return false;
                    }
                    break;

                default:
                    L_ERROR("UpdatMSGErr", "Optional attribute not recognized");

                    break;
            }

            if ((!PathAttribute::isFlagSet(
                    pathAtt.attributeTypeFlags,
                    PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL)) ||
                (PathAttribute::isFlagSet(
                    pathAtt.attributeTypeFlags,
                    PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE)) ||
                (!PathAttribute::isFlagSet(
                    pathAtt.attributeTypeFlags,
                    PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL))) {
            } else {
                *subcode = 4;
                L_ERROR("UpdatMSGErr", "The attribute has erroneous flags");
                pathAtt.getAttribute_be8(data_be8);

                return false;
            }
        }

        if (checkDoubleAtt[0] != true) {
            *subcode = 3;
            L_ERROR("UpdatMSGErr", "Missing Well-Known attribute");
            data_be8->push_back(
                PathAttribute::AttributeTypeCode_uint8_t::ORIGIN);

            return false;
        } else if (checkDoubleAtt[1] != true) {
            *subcode = 3;
            L_ERROR("UpdatMSGErr", "Missing Well-Known attribute");
            data_be8->push_back(
                PathAttribute::AttributeTypeCode_uint8_t::AS_PATH);

            return false;
        } else if (checkDoubleAtt[2] != true) {
            *subcode = 3;
            L_ERROR("UpdatMSGErr", "Missing Well-Known attribute");
            data_be8->push_back(
                PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP);

            return false;
        }
    }

    size_t nlri_len = getNetworkLayerReachabilityInfoBytesLength();

    if (nlri_len != 0) {
        size_t withdrawnRouteLen = getWithdrawnRoutesBytesLength();
        size_t pathAttrLen       = getPathAttributesBytesLength();

        size_t offset =
            sizeof(BGPCommonHeader)  // Space for: Common header Value
            + sizeof(uint16_t)       // Space for: Withdrawn Routes Length
            + withdrawnRouteLen      // Space for: Withdrawn Routes
            + sizeof(uint16_t)       // Space for: Total Path Attribute Length
            + pathAttrLen;           // Space for: Path Attributes

        uint8_t* dataPtr = m_Data + offset;

        // XXX Semantic correctness to ckeck in the message handling/parsing

        while (nlri_len > 0) {
            if (*dataPtr <= 8) {
                if (nlri_len < 2) {
                    *subcode = 10;
                    L_ERROR("UpdatMSGErr",
                            "NLRI length and prefix systactically incorrect");
                    return false;
                }
                nlri_len = nlri_len - 2;
                dataPtr  = dataPtr + 2;
            } else if (*dataPtr > 8 && *dataPtr <= 16) {
                if (nlri_len < 3) {
                    *subcode = 10;
                    L_ERROR("UpdatMSGErr",
                            "NLRI length and prefix systactically incorrect");
                    return false;
                }
                nlri_len = nlri_len - 3;
                dataPtr  = dataPtr + 3;
            } else if (*dataPtr > 16 && *dataPtr <= 24) {
                if (nlri_len < 4) {
                    *subcode = 10;
                    L_ERROR("UpdatMSGErr",
                            "NLRI length and prefix systactically incorrect");
                    return false;
                }
                nlri_len = nlri_len - 4;
                dataPtr  = dataPtr + 4;
            } else if (*dataPtr > 24 && *dataPtr <= 32) {
                if (nlri_len < 5) {
                    *subcode = 10;
                    L_ERROR("UpdatMSGErr",
                            "NLRI length and prefix systactically incorrect");
                    return false;
                }
                nlri_len = nlri_len - 5;
                dataPtr  = dataPtr + 5;
            } else {
                *subcode = 10;
                L_ERROR("UpdatMSGErr", "Invalid NLRI data");
                return false;
            }
        }
    }


    // TODO Well-known attributes not handled -> subcode 2
    // XXX: If it is not recognised, how do I know that it is well-known and
    // mandatory? Further, if it is not recognised it will be inserted in the
    // missing attributes

    //  If any of the well-known mandatory attributes are not recognized,
    // then the Error Subcode MUST be set to Unrecognized Well-known
    // Attribute.  The Data field MUST contain the unrecognized attribute
    // (type, length, and value).

    return true;
}

bool BGPUpdateLayer::checkNextHop(pcpp::IPv4Address recvAddress) {
    std::vector<PathAttribute> pathAttributes;

    getPathAttributes(pathAttributes);

    for (auto pathAtt : pathAttributes) {
        if (pathAtt.attributeTypeCode ==
            PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP) {
            uint8_t* ip = pathAtt.getAttributeValue_be();
            uint32_t ipToCheck =
                ip[0] | (ip[1] << 8) | (ip[2] << 16) | (ip[3] << 24);
            uint32_t recvIP = recvAddress.toInt();
            if (ipToCheck == recvIP) {
                L_ERROR("Verify IP",
                        "The IP is semantically incorrect -> It is the same as "
                        "the receiver IP");
                return false;
            }

            // XXX: Check further if semantically correct
            //    b) In the case of an EBGP, where the sender and receiver are
            //    one
            // IP hop away from each other, either the IP address in the
            // NEXT_HOP MUST be the sender's IP address that is used to
            // establish the BGP connection, or the interface associated with
            // the NEXT_HOP IP address MUST share a common subnet with the
            // receiving BGP speaker.
        }
    }
    return true;
}

bool BGPUpdateLayer::checkValidIP(uint32_t ip) {
    if (ip == 0x00000000) {
        L_ERROR("Verify IP", "The IP is invalid -> 0.0.0.0");
        return false;
    } else if (ip >= 0xE0000000 && ip <= 0xEFFFFFFF) {
        L_ERROR("Verify IP", "The IP is a multicast IP");
        return false;
    } else if ((ip >= 0x0A000000 && ip <= 0x0AFFFFFF) ||
               (ip >= 0xAC100000 && ip <= 0xAC10FFFF) ||
               (ip >= 0xC0A80000 && ip <= 0xC0A800FF)) {
        L_ERROR("Verify IP", "The IP is a private IP");
        return false;
    }
    return true;
}
