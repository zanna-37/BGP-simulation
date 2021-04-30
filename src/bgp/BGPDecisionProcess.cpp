#include <BGPDecisionProcess.h>

#include <algorithm>

#include "packets/BGPUpdatePathAttribute.h"

void startDecisionProcess(Router *router, BGPUpdateLayer *BGPUpdateMessage) {
    std::vector<BGPTableRow *> *BGPTable = router->bgpTable;
    // check if there are withdrawn routes
    if (BGPUpdateMessage->getWithdrawnRoutesBytesLength() != 0) {
        std::vector<LengthAndIpPrefix> withDrawnRoutes;
        BGPUpdateMessage->getWithdrawnRoutes(withDrawnRoutes);
        for (LengthAndIpPrefix withDrawnRoute : withDrawnRoutes) {
            for (BGPTableRow *BGPTableRoute : *BGPTable) {
                pcpp::IPv4Address networkIPwithDrawnRoute(
                    withDrawnRoute.ipPrefix.toInt() &
                    withDrawnRoute.prefixLength);

                if (networkIPwithDrawnRoute == BGPTableRoute->networkIP) {
                    // remove withdrawn route
                    BGPTable->erase(std::remove(BGPTable->begin(),
                                                BGPTable->end(),
                                                BGPTableRoute->networkIP),
                                    BGPTable->end());
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
                        origin = reinterpret_cast<char>(
                            pathAttribute.getAttributeValue_be());
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::AS_PATH:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        for (int i = 0; pathAttribute.getAttributeLength_h();
                             i++) {
                            uint16_t as_num =
                                (uint16_t)
                                    pathAttribute.getAttributeValue_be()[i];
                            asPath.push_back(as_num);
                        }

                        asPath = be16tho(asPath);
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP:
                    if (pathAttribute.getAttributeLength_h() > 31) {
                        uint32_t nextHopInt =
                            (uint32_t)pathAttribute.getAttributeValue_be();
                        nextHopInt = be32tho(nextHopInt);
                        nextHop    = pcpp::IPv4Address(nextHopInt);
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::LOCAL_PREF:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        localPreferences =
                            (uint32_t)pathAttribute.getAttributeValue_be();
                        localPreferences = be32tho(localPreferences);
                    }
                    break;
            }
        }

        for (LengthAndIpPrefix nlri : networkLayerReachabilityInfo) {
            for (BGPTableRow *BGPTableRoute : *BGPTable) {
                pcpp::IPv4Address networkIPNRLI(nlri.ipPrefix.toInt() &
                                                nlri.prefixLength);

                pcpp::IPv4Address netmaskNRLI(nlri.prefixLength);

                BGPTableRow *newRoute = new BGPTableRow(networkIPNRLI,
                                                        netmaskNRLI,
                                                        nextHop,
                                                        '?',
                                                        asPath,
                                                        0,
                                                        localPreferences,
                                                        0);

                if (networkIPNRLI == BGPTableRoute->networkIP) {
                    // TODO: use replace method
                    BGPTable->erase(std::remove(BGPTable->begin(),
                                                BGPTable->end(),
                                                BGPTableRoute->networkIP),
                                    BGPTable->end());
                    BGPTable->push_back(newRoute);

                } else {
                    calculatePreferredRoute(newRoute, BGPTableRoute);
                    BGPTable->push_back(newRoute);
                    if (newRoute->preferred) {
                        std::vector<TableRow *> *IPTable = router->routingTable;
                        updateIPTable(IPTable, newRoute);
                    }
                }
            }
        }
    }

    // SEND UPDATE MESSAGE
}

void updateIPTable(std::vector<TableRow *> *IPTable, BGPTableRow *newRoute) {
    TableRow *row = new TableRow(newRoute->networkIP,
                                 newRoute->networkMask,
                                 pcpp::IPv4Address::Zero,
                                 newRoute->networkCard->netInterface,
                                 newRoute->networkCard);
    IPTable->push_back(row);
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