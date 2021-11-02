#include "BGPStateEstablished.h"

#include <chrono>

#include "../../entities/Link.h"
#include "../../entities/Router.h"
#include "../../utils/SmartPointerUtils.h"
#include "../BGPApplication.h"
#include "../BGPConnection.h"
#include "../BGPDecisionProcess.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPKeepaliveLayer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPNotificationLayer.h"
#include "../packets/BGPUpdateLayer.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"


bool BGPStateEstablished ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventType) {
        case BGPEventType::ManualStart:
        case BGPEventType::AutomaticStart:
        case BGPEventType::AutomaticStart_with_PassiveTcpEstablishment:
        case BGPEventType::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventType::AutomaticStart_with_DampPeerOscillations:
        case BGPEventType::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            // (Events 1, 3-7) are ignored in the Active state.
            break;
        case BGPEventType::ManualStop:
            // sends the NOTIFICATION message with a Cease,

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::CEASE,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");

                // Create BGPUpdateMessage (no PathAttributes)
                pcpp::IPv4Address IPAddressPeer =
                    stateMachine->connection->dstAddr;
                pcpp::IPv4Address netMask;
                // L_DEBUG("IPAddressPeer", IPAddressPeer.toString());
                for (NetworkCard* networkCard :
                     *stateMachine->connection->owner->networkCards) {
                    // L_DEBUG("IP NETWORK CARD PEER",
                    // networkCardPeer->IP.toString());
                    if (networkCard->IP == stateMachine->connection->srcAddr) {
                        netMask = networkCard->netmask;
                        // L_DEBUG("IP NETCARD PEER",
                        // netMaskPeer.toString());
                    }
                }
                pcpp::IPv4Address networkIPpeer(IPAddressPeer.toInt() &
                                                netMask.toInt());

                std::vector<LengthAndIpPrefix> withdrawnRoutes;

                uint8_t prefLenPeer =
                    LengthAndIpPrefix::computeLengthIpPrefix(netMask);
                LengthAndIpPrefix withdrawnRoute(prefLenPeer,
                                                 networkIPpeer.toString());

                withdrawnRoutes.push_back(withdrawnRoute);

                std::vector<PathAttribute>     pathAttributes;
                std::vector<LengthAndIpPrefix> nlris;
                std::vector<uint16_t>          asPath;

                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnRoutes, pathAttributes, nlris);
                bgpUpdateLayer->computeCalculateFields();

                runDecisionProcess(stateMachine->connection->owner,
                                   bgpUpdateLayer,
                                   stateMachine->connection->srcAddr,
                                   stateMachine->connection);

                stateMachine->connection->bgpApplication->sendBGPUpdateMessage(
                    stateMachine->connection,
                    withdrawnRoutes,
                    asPath,
                    nlris,
                    false);
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero, and
            stateMachine->setConnectRetryCounter(0);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::AutomaticStop:
            // OPTIONAL
            // sends a NOTIFICATION with a Cease,

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::CEASE,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");
            }


            // sets the ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,
            // TODO copy from above

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::HoldTimer_Expires:
            // sends a NOTIFICATION message with the Error Code Hold Timer
            // Expired,

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::HOLD_TIMER_EXPIRED,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");

                // Create BGPUpdateMessage (no PathAttributes)
                pcpp::IPv4Address IPAddressPeer =
                    stateMachine->connection->dstAddr;
                pcpp::IPv4Address netMask;
                // L_DEBUG("IPAddressPeer", IPAddressPeer.toString());
                for (NetworkCard* networkCard :
                     *stateMachine->connection->owner->networkCards) {
                    // L_DEBUG("IP NETWORK CARD PEER",
                    // networkCardPeer->IP.toString());
                    if (networkCard->IP == stateMachine->connection->srcAddr) {
                        netMask = networkCard->netmask;
                        // L_DEBUG("IP NETCARD PEER",
                        // netMaskPeer.toString());
                    }
                }
                pcpp::IPv4Address networkIPpeer(IPAddressPeer.toInt() &
                                                netMask.toInt());

                std::vector<LengthAndIpPrefix> withdrawnRoutes;

                uint8_t prefLenPeer =
                    LengthAndIpPrefix::computeLengthIpPrefix(netMask);
                LengthAndIpPrefix withdrawnRoute(prefLenPeer,
                                                 networkIPpeer.toString());

                withdrawnRoutes.push_back(withdrawnRoute);

                std::vector<PathAttribute>     pathAttributes;
                std::vector<LengthAndIpPrefix> nlris;
                std::vector<uint16_t>          asPath;

                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnRoutes, pathAttributes, nlris);
                bgpUpdateLayer->computeCalculateFields();

                runDecisionProcess(stateMachine->connection->owner,
                                   bgpUpdateLayer,
                                   stateMachine->connection->srcAddr,
                                   stateMachine->connection);

                stateMachine->connection->bgpApplication->sendBGPUpdateMessage(
                    stateMachine->connection,
                    withdrawnRoutes,
                    asPath,
                    nlris,
                    false);
            }


            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::KeepaliveTimer_Expires:
            // sends a KEEPALIVE message, and

            {
                std::unique_ptr<BGPLayer> bgpKeepaliveLayer =
                    std::make_unique<BGPKeepaliveLayer>();
                bgpKeepaliveLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpKeepaliveLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending KEEPALIVE message");
            }

            // FIXME restarts its KeepaliveTimer, unless the negotiated HoldTime
            // value is zero. --> Should be correct now
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetKeepAliveTimer();
                stateMachine->keepAliveTimer->start();
            }
            break;

        case BGPEventType::TcpConnection_Valid:
            // OPTIONAL
            // XXX received for a valid port, will cause the second connection
            // to be tracked.

            handled = false;
            break;

        case BGPEventType::Tcp_CR_Invalid:

            break;

        case BGPEventType::Tcp_CR_Acked:
        case BGPEventType::TcpConnectionConfirmed:
            // MANDATORY  -- How?
            // TODO the second connection SHALL be tracked until it sends an
            // OPEN message.

            handled = false;
            break;

        case BGPEventType::BGPOpen:
            // If a valid OPEN message (BGPOpen (Event 19)) is received,
            // and if the CollisionDetectEstablishedState optional attribute is
            // TRUE, the OPEN message will be checked to see if it collides
            // (Section 6.8) with any other connection.

            if (stateMachine->getCollisionDetectEstablishedState()) {
                // OPTIONAL
            }

            // FIXME If the CollisionDetectEstablishedState is FALSE the RFC
            // does not give any instructions I suppose that the message needs
            // to be ignored
            break;

        case BGPEventType::OpenCollisionDump:
            // OPTIONAL
            // XXX sends a NOTIFICATION with a Cease,
            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::CEASE,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // XXX releases all BGP resources,
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::NotifMsgVerErr:
        case BGPEventType::NotifMsg:
        case BGPEventType::TcpConnectionFails:
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            {
                // Create BGPUpdateMessage (no PathAttributes)
                pcpp::IPv4Address IPAddressPeer =
                    stateMachine->connection->dstAddr;
                pcpp::IPv4Address netMask;
                // L_DEBUG("IPAddressPeer", IPAddressPeer.toString());
                for (NetworkCard* networkCard :
                     *stateMachine->connection->owner->networkCards) {
                    // L_DEBUG("IP NETWORK CARD PEER",
                    // networkCardPeer->IP.toString());
                    if (networkCard->IP == stateMachine->connection->srcAddr) {
                        netMask = networkCard->netmask;
                        // L_DEBUG("IP NETCARD PEER",
                        // netMaskPeer.toString());
                    }
                }
                pcpp::IPv4Address networkIPpeer(IPAddressPeer.toInt() &
                                                netMask.toInt());

                std::vector<LengthAndIpPrefix> withdrawnRoutes;

                uint8_t prefLenPeer =
                    LengthAndIpPrefix::computeLengthIpPrefix(netMask);
                LengthAndIpPrefix withdrawnRoute(prefLenPeer,
                                                 networkIPpeer.toString());

                withdrawnRoutes.push_back(withdrawnRoute);

                std::vector<PathAttribute>     pathAttributes;
                std::vector<LengthAndIpPrefix> nlris;
                std::vector<uint16_t>          asPath;

                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnRoutes, pathAttributes, nlris);
                bgpUpdateLayer->computeCalculateFields();

                runDecisionProcess(stateMachine->connection->owner,
                                   bgpUpdateLayer,
                                   stateMachine->connection->srcAddr,
                                   stateMachine->connection);

                stateMachine->connection->bgpApplication->sendBGPUpdateMessage(
                    stateMachine->connection,
                    withdrawnRoutes,
                    asPath,
                    nlris,
                    false);
            }

            // XXX releases all the BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::KeepAliveMsg:
            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.
            break;

        case BGPEventType::UpdateMsg: {
            // BGPUpdateMessage to be processed
            std::unique_ptr<BGPUpdateLayer> updateLayer;
            dynamic_pointer_move(updateLayer, event.layers);

            // Run Decision Process
            runDecisionProcess(stateMachine->connection->owner,
                               updateLayer,
                               stateMachine->connection->srcAddr,
                               stateMachine->connection);
        }

            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.
            break;

        case BGPEventType::UpdateMsgErr:
            // MANDATORY
            // sends a NOTIFICATION message with an Update error,
            // take the message error to insert in the notification

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::move(event.layers);

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,
            // table needed

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::ConnectRetryTimer_Expires:
        case BGPEventType::DelayOpenTimer_Expires:
        case BGPEventType::IdleHoldTimer_Expires:
        case BGPEventType::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventType::BGPHeaderErr:
        case BGPEventType::BGPOpenMsgErr:
            // sends a NOTIFICATION message with the Error Code Finite
            // State Machine Error,

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::FSM_ERR,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));

                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending NOTIFICATION message");

                // Create BGPUpdateMessage (no PathAttributes)
                pcpp::IPv4Address IPAddressPeer =
                    stateMachine->connection->dstAddr;
                pcpp::IPv4Address netMask;
                // L_DEBUG("IPAddressPeer", IPAddressPeer.toString());
                for (NetworkCard* networkCard :
                     *stateMachine->connection->owner->networkCards) {
                    // L_DEBUG("IP NETWORK CARD PEER",
                    // networkCardPeer->IP.toString());
                    if (networkCard->IP == stateMachine->connection->srcAddr) {
                        netMask = networkCard->netmask;
                        // L_DEBUG("IP NETCARD PEER",
                        // netMaskPeer.toString());
                    }
                }
                pcpp::IPv4Address networkIPpeer(IPAddressPeer.toInt() &
                                                netMask.toInt());

                std::vector<LengthAndIpPrefix> withdrawnRoutes;

                uint8_t prefLenPeer =
                    LengthAndIpPrefix::computeLengthIpPrefix(netMask);
                LengthAndIpPrefix withdrawnRoute(prefLenPeer,
                                                 networkIPpeer.toString());

                withdrawnRoutes.push_back(withdrawnRoute);

                std::vector<PathAttribute>     pathAttributes;
                std::vector<LengthAndIpPrefix> nlris;
                std::vector<uint16_t>          asPath;

                std::unique_ptr<BGPUpdateLayer> bgpUpdateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnRoutes, pathAttributes, nlris);
                bgpUpdateLayer->computeCalculateFields();

                runDecisionProcess(stateMachine->connection->owner,
                                   bgpUpdateLayer,
                                   stateMachine->connection->srcAddr,
                                   stateMachine->connection);

                stateMachine->connection->bgpApplication->sendBGPUpdateMessage(
                    stateMachine->connection,
                    withdrawnRoutes,
                    asPath,
                    nlris,
                    false);
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::SendUpdateMsg:
            // Event for checking that the fsm is in a good state before sending
            // the message

            {
                std::unique_ptr<BGPLayer> bgpUpdateLayer =
                    std::move(event.layers);

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpUpdateLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending UPDATE message");
            }

            break;

        case BGPEventType::MinASOriginationIntervalTimer_Expires:
            // sends Update message

            {
                bool        sameASPath = true;
                BGPTableRow previousBGPTableRow =
                    stateMachine->connection->owner->bgpTable[0];

                std::vector<LengthAndIpPrefix> new_nlri;
                uint8_t prefLen = LengthAndIpPrefix::computeLengthIpPrefix(
                    previousBGPTableRow.networkMask);
                LengthAndIpPrefix nlri(
                    prefLen, previousBGPTableRow.networkIP.toString());
                new_nlri.push_back(nlri);

                for (int i = 1;
                     i < stateMachine->connection->owner->bgpTable.size();
                     i++) {
                    if (previousBGPTableRow.asPath.size() ==
                        stateMachine->connection->owner->bgpTable[i]
                            .asPath.size()) {
                        for (int j = 0; j < previousBGPTableRow.asPath.size();
                             j++) {
                            if (previousBGPTableRow.asPath[j] !=
                                stateMachine->connection->owner->bgpTable[i]
                                    .asPath[j]) {
                                sameASPath = false;
                                break;
                            }
                        }
                        if (sameASPath ||
                            (!sameASPath && i == stateMachine->connection->owner
                                                         ->bgpTable.size() -
                                                     1)) {
                            uint8_t prefLen =
                                LengthAndIpPrefix::computeLengthIpPrefix(
                                    stateMachine->connection->owner->bgpTable[i]
                                        .networkMask);
                            LengthAndIpPrefix nlri(
                                prefLen,
                                stateMachine->connection->owner->bgpTable[i]
                                    .networkIP.toString());
                            new_nlri.push_back(nlri);
                        }
                    } else if (previousBGPTableRow.asPath.size() !=
                                   stateMachine->connection->owner->bgpTable[i]
                                       .asPath.size() &&
                               i == stateMachine->connection->owner->bgpTable
                                            .size() -
                                        1) {
                        sameASPath = false;
                        uint8_t prefLen =
                            LengthAndIpPrefix::computeLengthIpPrefix(
                                stateMachine->connection->owner->bgpTable[i]
                                    .networkMask);
                        LengthAndIpPrefix nlri(
                            prefLen,
                            stateMachine->connection->owner->bgpTable[i]
                                .networkIP.toString());
                        new_nlri.push_back(nlri);


                    } else {
                        sameASPath = false;
                    }

                    if (!sameASPath ||
                        i == stateMachine->connection->owner->bgpTable.size() -
                                 1) {
                        // Create BGPUpdateMessage
                        std::vector<PathAttribute> newPathAttributes;

                        // NextHop PathAttribute
                        uint32_t routerIp_int =
                            stateMachine->connection->srcAddr.toInt();
                        const size_t nextHopDataLength              = 4;
                        uint8_t      nextHopData[nextHopDataLength] = {
                            (uint8_t)routerIp_int,
                            (uint8_t)(routerIp_int >> 8),
                            (uint8_t)(routerIp_int >> 16),
                            (uint8_t)(routerIp_int >> 24)};
                        PathAttribute nextHopAttribute;
                        nextHopAttribute.setAttributeLengthAndValue(
                            nextHopData, nextHopDataLength);
                        nextHopAttribute.attributeTypeCode =
                            PathAttribute::AttributeTypeCode_uint8_t::NEXT_HOP;
                        nextHopAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL,
                            0);
                        nextHopAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::
                                TRANSITIVE,
                            1);
                        nextHopAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL,
                            0);
                        newPathAttributes.push_back(nextHopAttribute);

                        // AS_Path PathAttribute
                        std::vector<uint8_t> asPath_be8;

                        uint16_t new_as_num =
                            (uint16_t)
                                stateMachine->connection->owner->AS_number;

                        std::vector<uint16_t> asPath;

                        if (!sameASPath) {
                            asPath = previousBGPTableRow.asPath;
                        } else {
                            asPath =
                                stateMachine->connection->owner->bgpTable[i]
                                    .asPath;
                        }

                        asPath.push_back(new_as_num);

                        uint8_t asPathType = 2;
                        uint8_t asPathLen  = asPath.size();

                        PathAttribute::asPathToAttributeDataArray_be(
                            asPathType, asPathLen, asPath, asPath_be8);

                        size_t        asPathDataLength = asPath_be8.size();
                        uint8_t*      asPathData       = asPath_be8.data();
                        PathAttribute asPathAttribute;
                        asPathAttribute.setAttributeLengthAndValue(
                            asPathData, asPathDataLength);
                        asPathAttribute.attributeTypeCode =
                            PathAttribute::AttributeTypeCode_uint8_t::AS_PATH;
                        asPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL,
                            0);
                        asPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::
                                TRANSITIVE,
                            1);
                        asPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL,
                            0);
                        newPathAttributes.push_back(asPathAttribute);

                        // Origin PathAttribute
                        const size_t  originDataLength             = 1;
                        uint8_t       originData[originDataLength] = {2};
                        PathAttribute originPathAttribute;
                        originPathAttribute.setAttributeLengthAndValue(
                            originData, originDataLength);
                        originPathAttribute.attributeTypeCode =
                            PathAttribute::AttributeTypeCode_uint8_t::ORIGIN;
                        originPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::OPTIONAL,
                            0);
                        originPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::
                                TRANSITIVE,
                            1);
                        originPathAttribute.setFlags(
                            PathAttribute::AttributeTypeFlags_uint8_t::PARTIAL,
                            0);
                        newPathAttributes.push_back(originPathAttribute);

                        std::vector<LengthAndIpPrefix> withdrawnRoutes;

                        std::unique_ptr<BGPUpdateLayer> updateLayer =
                            std::make_unique<BGPUpdateLayer>(
                                withdrawnRoutes, newPathAttributes, new_nlri);
                        updateLayer->computeCalculateFields();

                        BGPEvent event = {BGPEventType::SendUpdateMsg,
                                          std::move(updateLayer)};
                        stateMachine->connection->enqueueEvent(
                            std::move(event));

                        L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                        stateMachine->name,
                                    stateMachine->connection->toString(),
                                    "Enqueuing UPDATE message in the events");

                        sameASPath = true;
                        previousBGPTableRow =
                            stateMachine->connection->owner->bgpTable[i];
                        new_nlri.clear();
                        uint8_t prefLen =
                            LengthAndIpPrefix::computeLengthIpPrefix(
                                previousBGPTableRow.networkMask);
                        LengthAndIpPrefix nlri(
                            prefLen, previousBGPTableRow.networkIP.toString());
                        new_nlri.push_back(nlri);
                    }
                }
            }

            // FIXME restarts its KeepaliveTimer, unless the negotiated
            // HoldTime value is zero. --> Should be correct now
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetMinASOriginationIntervalTimer();
                stateMachine->minASOriginationIntervalTimer->start();
            }
            break;

        default:
            handled = false;
            break;
    }

    return handled;
}

// TODO
//   Each time the local system sends a KEEPALIVE or UPDATE message, it
//   restarts its KeepaliveTimer, unless the negotiated HoldTime value
//   is zero.
