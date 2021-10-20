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
                if (withdrawnedNetworkIP == itTableRow->networkIP) {
                    // remove withdrawned route
                    uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                        itTableRow->networkMask);
                    LengthAndIpPrefix newWithDrawnRoute(
                        prefLen, withdrawnedNetworkIP.toString());
                    newWithDrawnRoutes.push_back(newWithDrawnRoute);
                    updateIPTable(router, *itTableRow, true);
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
                        /*uint32_t nextHopInt =
                            (uint32_t)*pathAttribute.getAttributeValue_be();*/
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

        std::vector<BGPTableRow> newRoutes;

        bool hasLoop = false;

        for (uint16_t as : asPath) {
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
                newRoutes.push_back(newRoute);
                for (auto itBGPTableRow = router->bgpTable.begin();
                     itBGPTableRow != router->bgpTable.end();) {
                    if (networkIPNRLI == itBGPTableRow->networkIP &&
                        nextHop == itBGPTableRow->nextHop) {
                        uint8_t prefLen =
                            LengthAndIpPrefix::computeLengthIpPrefix(
                                itBGPTableRow->networkMask);
                        LengthAndIpPrefix newWithDrawnRoute(
                            prefLen, itBGPTableRow->networkIP.toString());
                        newWithDrawnRoutes.push_back(newWithDrawnRoute);
                        updateIPTable(router, *itBGPTableRow, true);
                        itBGPTableRow = router->bgpTable.erase(itBGPTableRow);
                    } else {
                        ++itBGPTableRow;
                    }
                }
            }
        }

        std::vector<LengthAndIpPrefix> new_nlri;

        for (BGPTableRow newRoute : newRoutes) {
            router->bgpTable.push_back(newRoute);
            updateIPTable(router, newRoute, false);
            uint8_t prefLen =
                LengthAndIpPrefix::computeLengthIpPrefix(newRoute.networkMask);

            LengthAndIpPrefix nlri(prefLen, newRoute.networkIP.toString());
            new_nlri.push_back(nlri);
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


        if (!hasLoop) {
            asPath.push_back(new_as_num);
        }

        uint8_t asPathType = 2;  // TODO: Check that this is AS_SEQUENCE
        uint8_t asPathLen  = asPath.size();

        PathAttribute::asPathToAttributeDataArray_be(
            asPathType, asPathLen, asPath, asPath_be8);

        size_t        asPathDataLength = asPath_be8.size();
        uint8_t *     asPathData       = asPath_be8.data();
        PathAttribute asPathAttribute;
        asPathAttribute.setAttributeLengthAndValue(asPathData,
                                                   asPathDataLength);
        asPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
        asPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
        asPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
        asPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
        newPathAttributes.push_back(asPathAttribute);

        // Origin PathAttribute
        const size_t  originDataLength             = 1;
        uint8_t       originData[originDataLength] = {2};
        PathAttribute originPathAttribute;
        originPathAttribute.setAttributeLengthAndValue(originData,
                                                       originDataLength);
        originPathAttribute.attributeTypeCode =
            PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
        originPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL, 0);
        originPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::TRANSITIVE, 1);
        originPathAttribute.setFlags(
            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL, 0);
        newPathAttributes.push_back(originPathAttribute);

        // TODO LocalPreferences PathAttribute (if we have time)

        L_DEBUG(router->ID, "BGP Table:\n" + router->getBgpTableAsString());
        std::string routingTableAsString =
            IpManager::getRoutingTableAsString(router->routingTable);
        L_DEBUG(router->ID, "Routing Table:\n" + routingTableAsString);

        router->bgpApplication->sendBGPUpdateMessage(bgpConnectionToAvoid,
                                                     newWithDrawnRoutes,
                                                     newPathAttributes,
                                                     new_nlri);
    }
}

void updateIPTable(Router *router, BGPTableRow &route, bool isWithDrawnRoute) {
    if (isWithDrawnRoute) {
        for (auto itRoutingTable = router->routingTable.begin();
             itRoutingTable != router->routingTable.end();) {
            if (itRoutingTable->networkIP == route.networkIP &&
                itRoutingTable->defaultGateway == route.nextHop) {
                itRoutingTable = router->routingTable.erase(itRoutingTable);
            } else {
                ++itRoutingTable;
            }
        }
    } else {
        for (NetworkCard *networkCard : *router->networkCards) {
            NetworkCard *networkCardPeer =
                networkCard->link->getPeerNetworkCardOrNull(networkCard);
            if (networkCardPeer->IP == route.nextHop) {
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
