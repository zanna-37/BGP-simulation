#include "BGPDecisionProcess.h"

#include <endian.h>

#include <algorithm>

#include "../entities/Link.h"
#include "../entities/NetworkCard.h"
#include "../ip/IpManager.h"
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
    std::vector<LengthAndIpPrefix> new_nlris;

    if (BGPUpdateMessage->getWithdrawnRoutesBytesLength() != 0) {
        std::vector<LengthAndIpPrefix> withDrawnRoutes;
        BGPUpdateMessage->getWithdrawnRoutes(withDrawnRoutes);

        for (LengthAndIpPrefix withDrawnRoute : withDrawnRoutes) {
            pcpp::IPv4Address withdrawnedNetworkIP(
                withDrawnRoute.ipPrefix.toString());
            pcpp::IPv4Address withdrawnedNetworkMask(htobe32(
                NetUtils::prefixToNetmask(withDrawnRoute.prefixLength)));
            for (auto itTableRow = router->bgpTable.begin();
                 itTableRow != router->bgpTable.end();) {
                if (withdrawnedNetworkIP == itTableRow->networkIP &&
                    itTableRow->nextHop != pcpp::IPv4Address::Zero) {
                    // remove withdrawned route
                    uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                        itTableRow->networkMask);
                    LengthAndIpPrefix newWithDrawnRoute(
                        prefLen, itTableRow->networkIP.toString());
                    newWithDrawnRoutes.push_back(newWithDrawnRoute);
                    updateIPTable(router, *itTableRow, true, routerIP);
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
                        switch (*pathAttribute.getAttributeValue_be()) {
                            case 0:
                                origin = 'i';
                                break;
                            case 1:
                                origin = 'e';
                                break;
                            case 2:
                                origin = '?';
                                break;

                            default:
                                L_ERROR("DecisionProc",
                                        "Unhandled ORIGIN value " +
                                            std::to_string(
                                                *pathAttribute
                                                     .getAttributeValue_be()));
                                break;
                        }
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::AS_PATH:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        uint8_t *attributeValue_be =
                            pathAttribute.getAttributeValue_be();
                        size_t attributeValue_be_length =
                            pathAttribute.getAttributeLength_h();

                        uint8_t asType;

                        PathAttribute::attributeDataArray_beToAsPath(
                            attributeValue_be,
                            attributeValue_be_length,
                            asType,
                            asPath);
                    }
                    break;
                case PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP:
                    if (pathAttribute.getAttributeLength_h() > 0) {
                        uint8_t nextHopInt[4];
                        for (int i = 0;
                             i < pathAttribute.getAttributeLength_h();
                             i++) {
                            nextHopInt[i] =
                                (uint8_t) *
                                (pathAttribute.getAttributeValue_be() + i);
                        }
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

        bool hasLoop = false;

        for (uint16_t as : asPath) {
            L_DEBUG("ASPATH", std::to_string(as));
            if (as == router->AS_number) {
                hasLoop = true;
                break;
            }
        }

        if (!hasLoop) {
            for (LengthAndIpPrefix nlri : networkLayerReachabilityInfo) {
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
                newRoute.preferred = true;
                for (auto itBGPTableRow = router->bgpTable.begin();
                     itBGPTableRow != router->bgpTable.end();) {
                    if (networkIPNRLI == itBGPTableRow->networkIP &&
                        itBGPTableRow->nextHop != pcpp::IPv4Address::Zero &&
                        nextHop == itBGPTableRow->nextHop) {
                        uint8_t prefLen =
                            LengthAndIpPrefix::computeLengthIpPrefix(
                                itBGPTableRow->networkMask);
                        LengthAndIpPrefix newWithDrawnRoute(
                            prefLen, itBGPTableRow->networkIP.toString());
                        newWithDrawnRoutes.push_back(newWithDrawnRoute);
                        /*if (!itBGPTableRow->preferred) {
                            newRoute.preferred = false;
                        }*/
                        updateIPTable(router, *itBGPTableRow, true, routerIP);
                        itBGPTableRow = router->bgpTable.erase(itBGPTableRow);
                    } else {
                        if (networkIPNRLI == itBGPTableRow->networkIP &&
                            itBGPTableRow->nextHop != pcpp::IPv4Address::Zero &&
                            nextHop != itBGPTableRow->nextHop &&
                            itBGPTableRow->preferred) {
                            calculatePreferredRoute(newRoute, *itBGPTableRow);
                            if (itBGPTableRow->preferred) {
                                newRoute.preferred = false;
                            } else {
                                updateIPTable(
                                    router, *itBGPTableRow, true, routerIP);
                            }
                        }
                        ++itBGPTableRow;
                    }
                }
                router->bgpTable.push_back(newRoute);
                if (newRoute.preferred) {
                    updateIPTable(router, newRoute, false, routerIP);
                }
                uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                    newRoute.networkMask);

                LengthAndIpPrefix new_nlri(prefLen,
                                           newRoute.networkIP.toString());
                new_nlris.push_back(new_nlri);
            }

            // TODO LocalPreferences PathAttribute (if we have time)

            L_DEBUG(router->ID, "BGP Table:\n" + router->getBgpTableAsString());
            std::string routingTableAsString =
                IpManager::getRoutingTableAsString(router->routingTable);
            L_DEBUG(router->ID, "Routing Table:\n" + routingTableAsString);

            router->bgpApplication->sendBGPUpdateMessage(bgpConnectionToAvoid,
                                                         newWithDrawnRoutes,
                                                         asPath,
                                                         new_nlris,
                                                         true);
        }
    }
}

void updateIPTable(Router *           router,
                   BGPTableRow &      route,
                   bool               isWithDrawnRoute,
                   pcpp::IPv4Address &routerIP) {
    if (isWithDrawnRoute) {
        for (auto itRoutingTable = router->routingTable.begin();
             itRoutingTable != router->routingTable.end();) {
            if (itRoutingTable->networkIP == route.networkIP &&
                itRoutingTable->defaultGateway != pcpp::IPv4Address::Zero &&
                itRoutingTable->defaultGateway == route.nextHop) {
                itRoutingTable = router->routingTable.erase(itRoutingTable);
            } else {
                ++itRoutingTable;
            }
        }
    } else {
        for (NetworkCard *networkCard : *router->networkCards) {
            if (routerIP == networkCard->IP) {
                TableRow row(route.networkIP,
                             route.networkMask,
                             route.nextHop,
                             networkCard->netInterface,
                             networkCard);
                router->routingTable.emplace_back(row);
            }
        }
    }
}

void calculatePreferredRoute(BGPTableRow &newRoute,
                             BGPTableRow &currentPreferredRoute) {
    if (newRoute.asPath.size() < currentPreferredRoute.asPath.size()) {
        currentPreferredRoute.preferred = false;
    }
}
