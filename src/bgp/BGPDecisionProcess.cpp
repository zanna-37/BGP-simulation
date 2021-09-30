#include "BGPDecisionProcess.h"

#include <endian.h>

#include <algorithm>

#include "../logger/Logger.h"
#include "packets/BGPUpdatePathAttribute.h"

void runDecisionProcess(Router *                         router,
                        std::unique_ptr<BGPUpdateLayer> &BGPUpdateMessage,
                        std::unique_ptr<BGPUpdateLayer> &newBGPUpdateMessage,
                        pcpp::IPv4Address &              routerIP) {
    // check whether there are withdrawn routes
    if (BGPUpdateMessage->getWithdrawnRoutesBytesLength() != 0) {
        std::vector<LengthAndIpPrefix> withDrawnRoutes;
        BGPUpdateMessage->getWithdrawnRoutes(withDrawnRoutes);

        for (LengthAndIpPrefix withDrawnRoute : withDrawnRoutes) {
            pcpp::IPv4Address withdrawnedNetwork(
                withDrawnRoute.ipPrefix.toInt() & withDrawnRoute.prefixLength);

            for (auto itTableRow = router->bgpTable.begin();
                 itTableRow != router->bgpTable.end();) {
                if (withdrawnedNetwork == itTableRow->networkIP) {
                    // remove withdrawned route
                    itTableRow = router->bgpTable.erase(itTableRow);
                } else {
                    itTableRow++;
                }
            }
        }
    }

    // check if there are new routes
    if (BGPUpdateMessage->getPathAttributesBytesLength() > 0 &&
        BGPUpdateMessage->getNetworkLayerReachabilityInfoBytesLength() > 0) {
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

        std::vector<LengthAndIpPrefix> newWithDrawnRoutes;
        int                            jterator = 0;
        for (LengthAndIpPrefix nlri : networkLayerReachabilityInfo) {
            for (BGPTableRow &BGPTableRoute : router->bgpTable) {
                jterator++;
                pcpp::IPv4Address networkIPNRLI(nlri.ipPrefix.toInt() &
                                                nlri.prefixLength);
                pcpp::IPv4Address netmaskNRLI(nlri.prefixLength);
                BGPTableRow       newRoute(networkIPNRLI,
                                     netmaskNRLI,
                                     nextHop,
                                     origin,
                                     asPath,
                                     0,
                                     localPreferences,
                                     0);

                if (networkIPNRLI == BGPTableRoute.networkIP) {
                    // Replace table row
                    BGPTableRoute = newRoute;

                    newWithDrawnRoutes.emplace_back(
                        BGPTableRoute.networkMask.toInt(),
                        BGPTableRoute.networkIP.toString());
                } else {
                    calculatePreferredRoute(&newRoute, &BGPTableRoute);
                    router->bgpTable.push_back(newRoute);
                    if (newRoute.preferred) {
                        updateIPTable(router->routingTable, newRoute);

                        newWithDrawnRoutes.emplace_back(
                            BGPTableRoute.networkMask.toInt(),
                            BGPTableRoute.networkIP.toString());
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
        newPathAttributes.push_back(nextHopAttribute);

        // AS_Path PathAttribute
        uint16_t new_as_num = (uint16_t)router->AS_number;
        asPath.push_back(new_as_num);

        std::vector<uint8_t> asPath_be8;
        for (auto AS_h : asPath) {
            uint16_t AS_be16 = htobe16(AS_h);
            asPath_be8.push_back((uint8_t)AS_be16);
            asPath_be8.push_back((uint8_t)AS_be16 >> 8);
        }
        size_t        asPathDataLength = asPath_be8.size();
        uint8_t *     asPathData       = asPath_be8.data();
        PathAttribute asPathAttribute;
        asPathAttribute.setAttributeLengthAndValue(asPathData,
                                                   asPathDataLength);
        asPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
        newPathAttributes.push_back(asPathAttribute);

        // Origin PathAttribute
        const size_t  originDataLength             = 1;
        uint8_t       originData[originDataLength] = {'?'};
        PathAttribute originPathAttribute;
        originPathAttribute.setAttributeLengthAndValue(originData,
                                                       originDataLength);
        originPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
        newPathAttributes.push_back(originPathAttribute);

        // TODO LocalPreferences PathAttribute (if we have time)

        std::vector<LengthAndIpPrefix> new_nlri;
        for (BGPTableRow &bgpTableRow : router->bgpTable) {
            LengthAndIpPrefix nlri(bgpTableRow.networkMask.toInt(),
                                   bgpTableRow.networkIP.toString());
            new_nlri.push_back(nlri);
        }

        newBGPUpdateMessage = std::make_unique<BGPUpdateLayer>(
            newWithDrawnRoutes, newPathAttributes, new_nlri);
        newBGPUpdateMessage->computeCalculateFields();
    }
}

void updateIPTable(std::vector<TableRow> &IPTable, BGPTableRow &newRoute) {
    TableRow row(newRoute.networkIP,
                 newRoute.networkMask,
                 pcpp::IPv4Address::Zero,
                 newRoute.networkCard->netInterface,
                 newRoute.networkCard);
    IPTable.push_back(row);
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
