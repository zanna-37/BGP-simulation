#include "BGPDecisionProcess.h"

#include <endian.h>

#include <algorithm>

#include "../logger/Logger.h"
#include "../utils/NetUtils.h"
#include "BGPApplication.h"
#include "packets/BGPUpdatePathAttribute.h"

void runDecisionProcess(Router *                         router,
                        std::unique_ptr<BGPUpdateLayer> &BGPUpdateMessage,
                        pcpp::IPv4Address &              routerIP,
                        BGPConnection *                  bgpConnectionToAvoid) {
    // L_DEBUG("Decision Process", BGPUpdateMessage->toString());
    // check whether there are withdrawn routes
    std::vector<LengthAndIpPrefix> newWithDrawnRoutes;

    if (BGPUpdateMessage->getWithdrawnRoutesBytesLength() != 0) {
        L_DEBUG("Decision Process", "Processing withdrawn routes");
        std::vector<LengthAndIpPrefix> withDrawnRoutes;
        BGPUpdateMessage->getWithdrawnRoutes(withDrawnRoutes);

        for (LengthAndIpPrefix withDrawnRoute : withDrawnRoutes) {
            pcpp::IPv4Address withdrawnedNetworkIP(
                withDrawnRoute.ipPrefix.toString());
            pcpp::IPv4Address withdrawnedNetworkMask(htobe32(
                NetUtils::prefixToNetmask(withDrawnRoute.prefixLength)));
            for (auto itTableRow = router->bgpTable.begin();
                 itTableRow != router->bgpTable.end();) {
                if (withdrawnedNetworkIP == itTableRow->networkIP) {
                    // remove withdrawned route
                    uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                        itTableRow->networkMask);
                    LengthAndIpPrefix newWithDrawnRoute(
                        prefLen, withdrawnedNetworkIP.toString());
                    newWithDrawnRoutes.push_back(newWithDrawnRoute);

                    itTableRow = router->bgpTable.erase(itTableRow);
                    updateIPTable(router->routingTable, *itTableRow, true);
                } else {
                    itTableRow++;
                }
            }
        }
    }

    // check if there are new routes
    if (BGPUpdateMessage->getPathAttributesBytesLength() > 0 &&
        BGPUpdateMessage->getNetworkLayerReachabilityInfoBytesLength() > 0) {
        // L_DEBUG("Decision Process", "Checking new routes");
        std::vector<LengthAndIpPrefix> networkLayerReachabilityInfo;
        BGPUpdateMessage->getNetworkLayerReachabilityInfo(
            networkLayerReachabilityInfo);

        std::vector<PathAttribute> pathAttributes;
        BGPUpdateMessage->getPathAttributes(pathAttributes);

        char                  origin;
        std::vector<uint16_t> asPath;
        pcpp::IPv4Address     nextHop;
        uint32_t              localPreferences;

        for (PathAttribute pathAttribute : pathAttributes) {
            switch (pathAttribute.attributeTypeCode) {
                case PathAttribute::AttributeTypeCode_uint8_t::ORIGIN:
                    if (pathAttribute.getAttributeLength_h() != 0) {
                        // origin attribute is one byte, no need to
                        // do conversion to host byte order
                        origin = static_cast<char>(
                            *pathAttribute.getAttributeValue_be());
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::AS_PATH:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        for (int i = 0;
                             i < pathAttribute.getAttributeLength_h();
                             i += 2) {
                            uint16_t as_num = (uint16_t)be16toh(
                                *(pathAttribute.getAttributeValue_be() + i));
                            asPath.push_back(as_num);
                        }
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP:
                    if (pathAttribute.getAttributeLength_h() > 31) {
                        uint32_t nextHopInt = (uint32_t)be32toh(
                            *pathAttribute.getAttributeValue_be());
                        nextHop = pcpp::IPv4Address(nextHopInt);
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::LOCAL_PREF:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        localPreferences = (uint32_t)be32toh(
                            *pathAttribute.getAttributeValue_be());
                    }
                    break;
                default:
                    L_ERROR(
                        "DecisionProc",
                        "Unhandled PathAttribute::AttributeTypeCode_uint8_t " +
                            std::to_string(pathAttribute.attributeTypeCode));
                    break;
            }
        }

        for (LengthAndIpPrefix nlri : networkLayerReachabilityInfo) {
            for (BGPTableRow &BGPTableRoute : router->bgpTable) {
                if (BGPTableRoute.networkIP != router->loopbackIP) {
                    pcpp::IPv4Address networkIPNRLI(nlri.ipPrefix.toString());
                    pcpp::IPv4Address netmaskNRLI(
                        htobe32(NetUtils::prefixToNetmask(nlri.prefixLength)));
                    BGPTableRow newRoute(networkIPNRLI,
                                         netmaskNRLI,
                                         nextHop,
                                         origin,
                                         asPath,
                                         0,
                                         localPreferences,
                                         0);
                    if (networkIPNRLI != BGPTableRoute.networkIP) {
                        calculatePreferredRoute(&newRoute, &BGPTableRoute);
                        router->bgpTable.push_back(newRoute);
                        if (newRoute.preferred) {
                            updateIPTable(
                                router->routingTable, newRoute, false);

                            newWithDrawnRoutes.emplace_back(
                                BGPTableRoute.networkMask.toInt(),
                                BGPTableRoute.networkIP.toString());
                        }
                    }
                }
            }
        }

        // Create BGPUpdateMessage
        std::vector<PathAttribute> newPathAttributes;

        // NextHop PathAttribute
        uint32_t     routerIp_int                   = routerIP.toInt();
        const size_t nextHopDataLength              = 4;
        uint8_t      nextHopData[nextHopDataLength] = {
            (uint8_t)routerIp_int,
            (uint8_t)(routerIp_int >> 8),
            (uint8_t)(routerIp_int >> 16),
            (uint8_t)(routerIp_int >> 24)};
        PathAttribute nextHopAttribute;
        nextHopAttribute.setAttributeLengthAndValue(nextHopData,
                                                    nextHopDataLength);
        nextHopAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
        newPathAttributes.push_back(nextHopAttribute);

        // AS_Path PathAttribute
        std::vector<uint8_t> asPath_be8;
        uint16_t             new_as_num = (uint16_t)router->AS_number;

        bool isASPath = false;

        for (int i = 0; i < asPath.size(); i++) {
            if (new_as_num == asPath[i]) {
                isASPath = true;
            }
        }

        if (!isASPath) {
            asPath.push_back(new_as_num);
        }

        uint8_t asPathType = 2;  // TODO: Check that this is AS_SEQUENCE
        uint8_t asPathLen  = asPath.size();

        PathAttribute::buildAsPathAttributeData_be(
            asPathType, asPathLen, asPath, asPath_be8);

        size_t        asPathDataLength = asPath_be8.size();
        uint8_t *     asPathData       = asPath_be8.data();
        PathAttribute asPathAttribute;
        asPathAttribute.setAttributeLengthAndValue(asPathData,
                                                   asPathDataLength);
        asPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
        newPathAttributes.push_back(asPathAttribute);

        // Origin PathAttribute
        const size_t  originDataLength             = 1;
        uint8_t       originData[originDataLength] = {'?'};
        PathAttribute originPathAttribute;
        originPathAttribute.setAttributeLengthAndValue(originData,
                                                       originDataLength);
        originPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
        nextHopAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
        newPathAttributes.push_back(originPathAttribute);

        // TODO LocalPreferences PathAttribute (if we have time)
        std::vector<LengthAndIpPrefix> new_nlri;

        for (BGPTableRow &bgpTableRow : router->bgpTable) {
            if (router->loopbackIP != bgpTableRow.networkIP) {
                uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                    bgpTableRow.networkMask);

                LengthAndIpPrefix nlri(prefLen,
                                       bgpTableRow.networkIP.toString());
                new_nlri.push_back(nlri);
            }
        }

        router->bgpApplication->sendBGPUpdateMessage(bgpConnectionToAvoid,
                                                     newWithDrawnRoutes,
                                                     newPathAttributes,
                                                     new_nlri);
    }
}

void updateIPTable(std::vector<TableRow> &IPTable,
                   BGPTableRow &          route,
                   bool                   isWithDrawnRoute) {
    if (isWithDrawnRoute) {
        for (auto itRoutingTable = IPTable.begin();
             itRoutingTable != IPTable.end();) {
            if (itRoutingTable->networkIP == route.networkIP) {
                itRoutingTable = IPTable.erase(itRoutingTable);
            } else {
                ++itRoutingTable;
            }
        }
    } else {
        TableRow row(route.networkIP,
                     route.networkMask,
                     pcpp::IPv4Address::Zero,
                     route.networkCard->netInterface,
                     route.networkCard);
        IPTable.push_back(row);
    }
}

void calculatePreferredRoute(BGPTableRow *newRoute,
                             BGPTableRow *currentPreferredRoute) {
    if (newRoute->weight > currentPreferredRoute->weight) {
        currentPreferredRoute->preferred = false;
    } else if (newRoute->weight == currentPreferredRoute->weight &&
               newRoute->asPath.size() < currentPreferredRoute->asPath.size()) {
        currentPreferredRoute->preferred = false;
    } else {
        newRoute->preferred = false;
    }
}
