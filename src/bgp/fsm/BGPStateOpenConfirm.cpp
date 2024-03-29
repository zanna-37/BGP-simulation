#include "BGPStateOpenConfirm.h"

#include "../../entities/Router.h"
#include "../../utils/SmartPointerUtils.h"
#include "../BGPApplication.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPKeepaliveLayer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPNotificationLayer.h"
#include "../packets/BGPOpenLayer.h"
#include "../packets/BGPUpdateLayer.h"
#include "../packets/BGPUpdatePathAttribute.h"
#include "BGPStateEstablished.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"


bool BGPStateOpenConfirm ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventType) {
        case BGPEventType::ManualStart:
        case BGPEventType::AutomaticStart:
        case BGPEventType::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventType::AutomaticStart_with_PassiveTcpEstablishment:
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
            }

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // sets the ConnectRetryTimer to zero, and
            stateMachine->resetConnectRetryTimer();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::AutomaticStop:
            // OPTIONAL
            // XXX remove if not necessary
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

            handled = false;
            break;

        case BGPEventType::HoldTimer_Expires:
            // sends the NOTIFICATION message with the Error Code Hold
            // Timer Expired,

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
            // sends a KEEPALIVE message,

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

            // restarts the KeepaliveTimer
            stateMachine->resetKeepAliveTimer();
            stateMachine->keepAliveTimer->start();

            // and remains in the OpenConfirmed state.
            break;

        case BGPEventType::TcpConnection_Valid:
        case BGPEventType::Tcp_CR_Acked:
        case BGPEventType::TcpConnectionConfirmed:
            // TODO MANDATORY TO BE DONE! but How?
            // TODO the local system needs to track the second connection.

            handled = false;
            break;

        case BGPEventType::Tcp_CR_Invalid:
            // OPTIONAL
            // XXX the local system will ignore the second connection attempt.

            handled = false;
            break;

        case BGPEventType::TcpConnectionFails:
        case BGPEventType::NotifMsg:
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

        case BGPEventType::NotifMsgVerErr:
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::BGPOpen:
            // DONE MANDATORY TO BE DONE!
            // DONE If this connection is to be dropped due to connection
            // collision, the local system:

            // And if it does not need to be dropped? The message is ignored??

            // DONE maybe the collision detection should work in some
            // other way


            // sends a NOTIFICATION with a Cease inside the Collision Detection
            // method,
            {
                std::unique_ptr<BGPOpenLayer> openLayer;
                dynamic_pointer_move(openLayer, event.layers);

                pcpp::IPv4Address bgpIdentifier = pcpp::IPv4Address(
                    openLayer->getOpenHeaderOrNull()->BGPIdentifier_be);

                stateMachine->connection->bgpApplication->collisionDetection(
                    stateMachine->connection, bgpIdentifier);
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources,
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection (send TCP FIN),
            stateMachine->connection->dropConnection(true);

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
            break;

        case BGPEventType::BGPHeaderErr:
        case BGPEventType::BGPOpenMsgErr:
            // MANDATORY
            // sends a NOTIFICATION message with the appropriate error code

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

            // XXX releases all BGP resources
            // stateMachine->connection->bgpApplication->stopListeningOnSocket(
            //     stateMachine->connection->srcAddr);

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
            break;

        case BGPEventType::KeepAliveMsg:
            // TODO check if it works
            // FIXME restarts the HoldTimer and
            stateMachine->resetHoldTimer();
            stateMachine->holdTimer->start();

            // changes its state to Established.
            stateMachine->changeState(new BGPStateEstablished(stateMachine));

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

                // XXX LocalPreferences PathAttribute (if we have time)

                /*std::vector<LengthAndIpPrefix> withdrawnRoutes;

                std::unique_ptr<BGPUpdateLayer> updateLayer =
                    std::make_unique<BGPUpdateLayer>(
                        withdrawnRoutes, newPathAttributes, new_nlri);
                updateLayer->computeCalculateFields();


                // Enqueue new BGPUpdateMessage
                {
                    BGPEvent event = {BGPEventType::SendUpdateMsg,
                                      std::move(updateLayer)};
                    stateMachine->connection->enqueueEvent(std::move(event));

                    L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                               stateMachine->name,
                               stateMachine->connection->toString(),
                               "Enqueuing UPDATE message in the events");
                }*/
            }

            break;

        case BGPEventType::ConnectRetryTimer_Expires:
        case BGPEventType::DelayOpenTimer_Expires:
        case BGPEventType::IdleHoldTimer_Expires:
        case BGPEventType::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventType::UpdateMsg:
        case BGPEventType::UpdateMsgErr:
            // sends a NOTIFICATION with a code of Finite State Machine
            // Error

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
            L_ERROR_CONN(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                stateMachine->connection->toString(),
                "UPDATE message cannot be sent in OpenConfirm state");
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
