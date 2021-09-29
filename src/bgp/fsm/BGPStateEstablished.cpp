#include "BGPStateEstablished.h"

#include <chrono>

#include "../../entities/Router.h"
#include "../../utils/SmartPointerUtils.h"
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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> ManualStart, AutomaticStart, "
                    "ManualStart_with_PassiveTcpEstablishment, "
                    "AutomaticStart_with_PassiveTcpEstablishment, "
                    "AutomaticStart_with_DampPeerOscillations, "
                    "AutomaticStart_with_DampPeerOscillations_and_"
                    "PassiveTcpEstablishment");
            break;
        case BGPEventType::ManualStop:
            L_DEBUG(stateMachine->connection->owner->ID, "Event -> ManualStop");
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
                L_INFO(stateMachine->connection->owner->ID + " " +
                           stateMachine->name,
                       "Sending NOTIFICATION message");
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,
            // TODO BGP table needed

            // XXX releases BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero, and
            stateMachine->setConnectRetryCounter(0);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::AutomaticStop:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> AutomaticStop");
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
                L_INFO(stateMachine->connection->owner->ID + " " +
                           stateMachine->name,
                       "Sending NOTIFICATION message");
            }


            // sets the ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // XXX deletes all routes associated with this connection,
            // XXX copy from above

            // XXX releases all BGP resources, done

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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> HoldTimer_Expires");
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
                L_INFO(stateMachine->connection->owner->ID + " " +
                           stateMachine->name,
                       "Sending NOTIFICATION message");
            }


            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> KeepaliveTimer_Expires");
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
                L_INFO(stateMachine->connection->owner->ID + " " +
                           stateMachine->name,
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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> TcpConnection_Valid");
            // OPTIONAL
            // XXX received for a valid port, will cause the second connection
            // to be tracked.

            handled = false;
            break;

        case BGPEventType::Tcp_CR_Invalid:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> Tcp_CR_Invalid");

            break;

        case BGPEventType::Tcp_CR_Acked:
        case BGPEventType::TcpConnectionConfirmed:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> Tcp_CR_Acked, TcpConnectionConfirmed");
            // MANDATORY  -- How?
            // TODO the second connection SHALL be tracked until it sends an
            // OPEN message.

            handled = false;
            break;

        case BGPEventType::BGPOpen:
            L_DEBUG(stateMachine->connection->owner->ID, "Event -> BGPOpen");

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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> OpenCollisionDump");
            // OPTIONAL
            // XXX sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX deletes all routes associated with this connection,

            // XXX releases all BGP resources,

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
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> NotifMsgVerErr, NotifMsg, TcpConnectionFails");
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            {
                std::unique_ptr<BGPUpdateLayer> updateLayer;
                dynamic_pointer_move(updateLayer, event.layers);

                // Run Decision Process
                std::unique_ptr<BGPUpdateLayer> newUpdateLayer;
                runDecisionProcess(stateMachine->connection->owner,
                                   updateLayer,
                                   newUpdateLayer,
                                   stateMachine->connection->dstAddr);

                // Send new BGPUpdateMessage
                if (newUpdateLayer != nullptr) {
                    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                        layers = make_unique<
                            std::stack<std::unique_ptr<pcpp::Layer>>>();
                    layers->push(std::move(newUpdateLayer));

                    stateMachine->connection->sendData(std::move(layers));

                    L_INFO(stateMachine->connection->owner->ID + " " +
                               stateMachine->name,
                           "Sending UPDATE message");
                }
            }


            // XXX releases all the BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventType::KeepAliveMsg:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> KeepAliveMsg");
            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.
            break;

        case BGPEventType::UpdateMsg:
            L_DEBUG(stateMachine->connection->owner->ID, "Event -> UpdateMsg");
            {
                // BGPUpdateMessage to be processed
                std::unique_ptr<BGPUpdateLayer> updateLayer;
                dynamic_pointer_move(updateLayer, event.layers);

                // Run Decision Process
                std::unique_ptr<BGPUpdateLayer> newUpdateLayer;
                runDecisionProcess(stateMachine->connection->owner,
                                   updateLayer,
                                   newUpdateLayer,
                                   stateMachine->connection->dstAddr);

                // Send new BGPUpdateMessage
                if (newUpdateLayer != nullptr) {
                    std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                        layers = make_unique<
                            std::stack<std::unique_ptr<pcpp::Layer>>>();
                    layers->push(std::move(newUpdateLayer));

                    stateMachine->connection->sendData(std::move(layers));

                    L_INFO(stateMachine->connection->owner->ID + " " +
                               stateMachine->name,
                           "Sending UPDATE message");
                }
            }


            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.

            handled = false;
            break;

        case BGPEventType::UpdateMsgErr:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> UpdateMsgErr");
            // MANDATORY
            // sends a NOTIFICATION message with an Update error,
            // take the message error to insert in the notification

            // FIXME the error of the UPDATE message is not well computed yet
            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::move(event.layers);

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO(
                    stateMachine->connection->owner->ID + " " +
                        stateMachine->name,
                    "Sending NOTIFICATION message FIXME -> No uptdate error");
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,
            // table needed

            // XXX releases all BGP resources, done

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
            L_DEBUG(
                stateMachine->connection->owner->ID,
                "Event -> ConnectRetryTimer_Expires, DelayOpenTimer_Expires, "
                "IdleHoldTimer_Expires, BGPOpen_with_DelayOpenTimer_running, "
                "BGPHeaderErr, BGPOpenMsgErr");
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
                L_INFO(stateMachine->connection->owner->ID + " " +
                           stateMachine->name,
                       "Sending NOTIFICATION message");
            }

            // TODO deletes all routes associated with this connection,
            // bgp table needed

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

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
