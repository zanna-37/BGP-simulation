#include "BGPDecisionProcess.h"

#include <endian.h>

#include <algorithm>

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
                        origin = *pathAttribute.getAttributeValue_be();
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

                BGPTableRow newRoute(networkIPNRLI,
                                     netmaskNRLI,
                                     nextHop,
                                     '?',
                                     asPath,
                                     0,
                                     localPreferences,
                                     0);

                if (networkIPNRLI == BGPTableRoute.networkIP) {
                    // TODO: use replace method (not mandatory)
                    router->bgpTable.erase(router->bgpTable.begin() + jterator);
                    router->bgpTable.push_back(newRoute);

                    LengthAndIpPrefix newWithDrawnRoute(
                        BGPTableRoute.networkMask.toInt(),
                        BGPTableRoute.networkIP.toString());
                    newWithDrawnRoutes.push_back(newWithDrawnRoute);

                } else {
                    calculatePreferredRoute(&newRoute, &BGPTableRoute);
                    router->bgpTable.push_back(newRoute);
                    if (newRoute.preferred) {
                        updateIPTable(router->routingTable, newRoute);

                        LengthAndIpPrefix newWithDrawnRoute(
                            BGPTableRoute.networkMask.toInt(),
                            BGPTableRoute.networkIP.toString());
                        newWithDrawnRoutes.push_back(newWithDrawnRoute);
                    }
                }
            }
        }

        // Create BGPUpdateMessage
        std::vector<PathAttribute> newPathAttributes;


        // NextHop PathAttribute
        uint32_t      newNextHop        = routerIP.toInt();
        uint8_t *     nextHopData       = (uint8_t *)&newNextHop;
        uint16_t      nextHopDataLength = sizeof(newNextHop);
        PathAttribute nextHopAttribute;
        nextHopAttribute.setAttributeLengthAndValue(nextHopData,
                                                    nextHopDataLength);
        nextHopAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
        newPathAttributes.push_back(nextHopAttribute);

        // AS_Path PathAttribute
        uint16_t new_as_num = (uint16_t)router->AS_number;
        asPath.push_back(new_as_num);
        uint8_t *     asPathData       = (uint8_t *)&asPath;
        uint16_t      asPathDataLength = asPath.size();
        PathAttribute asPathAttribute;
        asPathAttribute.setAttributeLengthAndValue(asPathData,
                                                   asPathDataLength);
        asPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
        newPathAttributes.push_back(asPathAttribute);

        // Origin PathAttribute
        char          newOrigin        = '?';
        uint8_t *     originData       = (uint8_t *)&newOrigin;
        uint16_t      originDataLength = sizeof(newOrigin);
        PathAttribute originPathAttribute;
        originPathAttribute.setAttributeLengthAndValue(originData,
                                                       originDataLength);
        originPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
        newPathAttributes.push_back(originPathAttribute);

        // LocalPreferences PathAttribute
        uint32_t      newLocalPreferences  = 0;
        uint8_t *     localPreferencesData = (uint8_t *)&newLocalPreferences;
        uint16_t      localPreferencesDataLength = sizeof(newLocalPreferences);
        PathAttribute localPreferencesPathAttribute;
        localPreferencesPathAttribute.setAttributeLengthAndValue(
            localPreferencesData, localPreferencesDataLength);
        localPreferencesPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::LOCAL_PREF;
        newPathAttributes.push_back(localPreferencesPathAttribute);

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