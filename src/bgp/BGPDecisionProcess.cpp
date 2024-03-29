#include "BGPDecisionProcess.h"

#include <endian.h>

#include <algorithm>
#include <cassert>

#include "../entities/Link.h"
#include "../entities/NetworkCard.h"
#include "../ip/IpManager.h"
#include "../logger/Logger.h"
#include "../utils/NetUtils.h"
#include "BGPApplication.h"
#include "packets/BGPUpdatePathAttribute.h"

void runDecisionProcess(Router *                         router,
                        std::unique_ptr<BGPUpdateLayer> &BGPUpdateMessage,
                        pcpp::IPv4Address &              localRouterIP,
                        BGPConnection *                  bgpConnectionToAvoid) {
    L_DEBUG_CONN(
        "Decision Process", bgpConnectionToAvoid->toString(), "Running");
    // L_DEBUG("Decision Process", BGPUpdateMessage->toString());
    std::vector<LengthAndIpPrefix> newWithDrawnRoutes;
    std::vector<LengthAndIpPrefix> new_nlris;

    // Check whether there are withdrawn routes
    if (BGPUpdateMessage->getWithdrawnRoutesBytesLength() != 0) {
        std::vector<LengthAndIpPrefix> withDrawnRoutes;
        BGPUpdateMessage->getWithdrawnRoutes(withDrawnRoutes);

        // For every WithdrawnRoutes received
        for (LengthAndIpPrefix withDrawnRoute : withDrawnRoutes) {
            pcpp::IPv4Address withdrawnedNetworkIP(
                withDrawnRoute.ipPrefix.toString());
            pcpp::IPv4Address withdrawnedNetworkMask(htobe32(
                NetUtils::prefixToNetmask(withDrawnRoute.prefixLength)));

            for (auto itTableRow = router->bgpTable.begin();
                 itTableRow != router->bgpTable.end();) {
                if (withdrawnedNetworkIP == itTableRow->networkIP &&
                    itTableRow->nextHop != pcpp::IPv4Address::Zero &&
                    itTableRow->nextHop == bgpConnectionToAvoid->dstAddr) {
                    // Push the withdrownRoute for the subsequent BGP Update
                    // sent to others
                    uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                        itTableRow->networkMask);
                    LengthAndIpPrefix newWithDrawnRoute(
                        prefLen, itTableRow->networkIP.toString());
                    newWithDrawnRoutes.push_back(newWithDrawnRoute);

                    // Remove withdrawned route
                    updateIPTable(router, *itTableRow, true);
                    if (itTableRow->preferred) {
                        // We are deleting a preferred route so we should search
                        // a substitute
                        BGPTableRow *candidateSubstituteSoFar = nullptr;
                        for (auto &rowToCheck : router->bgpTable) {
                            if (/*skip self*/ &(*itTableRow) != &rowToCheck &&
                                itTableRow->networkIP == rowToCheck.networkIP &&
                                itTableRow->networkMask ==
                                    rowToCheck.networkMask &&
                                itTableRow->nextHop !=
                                    pcpp::IPv4Address::Zero) {
                                if (candidateSubstituteSoFar == nullptr ||
                                    rowToCheck.asPath <
                                        candidateSubstituteSoFar->asPath) {
                                    candidateSubstituteSoFar = &rowToCheck;
                                }
                            }
                        }
                        if (candidateSubstituteSoFar) {
                            // If we found a substitute, use it
                            candidateSubstituteSoFar->preferred = true;
                            updateIPTable(
                                router, *candidateSubstituteSoFar, false);
                        }
                    }
                    itTableRow = router->bgpTable.erase(itTableRow);
                    // END: Remove withdrawned route
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

                // Check if the route is a duplicate
                bool newRouteIsDuplicate = false;
                for (const auto &bgpTableRow : router->bgpTable) {
                    if (newRoute.networkIP == bgpTableRow.networkIP &&
                        newRoute.networkMask == bgpTableRow.networkMask &&
                        newRoute.nextHop == bgpTableRow.nextHop &&
                        newRoute.asPath == bgpTableRow.asPath) {
                        newRouteIsDuplicate = true;
                    }
                }

                if (!newRouteIsDuplicate) {
                    for (auto itBGPTableRow = router->bgpTable.begin();
                         itBGPTableRow != router->bgpTable.end();) {
                        if (networkIPNRLI == itBGPTableRow->networkIP &&
                            itBGPTableRow->nextHop != pcpp::IPv4Address::Zero &&
                            itBGPTableRow->preferred) {
                            dispreferWorstRoute(newRoute, *itBGPTableRow);
                            if (newRoute.preferred) {
                                // Remove old (now dispreferred) route
                                updateIPTable(router, *itBGPTableRow, true);
                            }
                        }
                        ++itBGPTableRow;
                    }

                    router->bgpTable.push_back(newRoute);

                    if (newRoute.preferred) {
                        // Add new route if found to be preferred
                        updateIPTable(router, newRoute, false);
                    }
                }

                // Push the newRoute for the subsequent BGP Update sent to
                // others
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

void updateIPTable(Router *router, BGPTableRow &route, bool isWithDrawnRoute) {
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
        // Workaround
        if (route.nextHop != pcpp::IPv4Address::Zero) {
            NetworkCard *exitingNetworkCard =
                router->getNextHopNetworkCardOrNull(route.nextHop);
            TableRow row(route.networkIP,
                         route.networkMask,
                         route.nextHop,
                         exitingNetworkCard->netInterface,
                         exitingNetworkCard);
            router->routingTable.emplace_back(row);
        }
    }
}

void dispreferWorstRoute(BGPTableRow &newRoute,
                         BGPTableRow &currentPreferredRoute) {
    assert(newRoute.preferred);
    assert(currentPreferredRoute.preferred);

    if (newRoute.asPath.size() < currentPreferredRoute.asPath.size()) {
        currentPreferredRoute.preferred = false;
    } else {
        newRoute.preferred = false;
    }
}
