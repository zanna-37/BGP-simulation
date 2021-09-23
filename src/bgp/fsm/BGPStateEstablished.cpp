#include "BGPStateEstablished.h"

#include <chrono>

#include "../../entities/Router.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPKeepaliveLayer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPNotificationLayer.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"


bool BGPStateEstablished ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventList) {
        case BGPEventList::ManualStart:
        case BGPEventList::AutomaticStart:
        case BGPEventList::AutomaticStart_with_PassiveTcpEstablishment:
        case BGPEventList::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventList::AutomaticStart_with_DampPeerOscillations:
        case BGPEventList::
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
        case BGPEventList::ManualStop:
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

        case BGPEventList::AutomaticStop:
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

        case BGPEventList::HoldTimer_Expires:
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

        case BGPEventList::KeepaliveTimer_Expires:
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

        case BGPEventList::TcpConnection_Valid:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> TcpConnection_Valid");
            // OPTIONAL
            // XXX received for a valid port, will cause the second connection
            // to be tracked.

            handled = false;
            break;

        case BGPEventList::Tcp_CR_Invalid:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> Tcp_CR_Invalid");

            break;

        case BGPEventList::Tcp_CR_Acked:
        case BGPEventList::TcpConnectionConfirmed:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> Tcp_CR_Acked, TcpConnectionConfirmed");
            // MANDATORY  -- How?
            // TODO the second connection SHALL be tracked until it sends an
            // OPEN message.

            handled = false;
            break;

        case BGPEventList::BGPOpen:
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

        case BGPEventList::OpenCollisionDump:
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

        case BGPEventList::NotifMsgVerErr:
        case BGPEventList::NotifMsg:
        case BGPEventList::TcpConnectionFails:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> NotifMsgVerErr, NotifMsg, TcpConnectionFails");
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,
            // bgp table needed


            // XXX releases all the BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventList::KeepAliveMsg:
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

        case BGPEventList::UpdateMsg:
            L_DEBUG(stateMachine->connection->owner->ID, "Event -> UpdateMsg");
            // MANDATORY
            // TODO processes the message,

            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getNegotiatedHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.

            handled = false;
            break;

        case BGPEventList::UpdateMsgErr:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> UpdateMsgErr");
            // MANDATORY
            // sends a NOTIFICATION message with an Update error,
            // take the message error to insert in the notification

            // FIXME the error of the UPDATE message is not well computed yet
            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer(
                    dynamic_cast<BGPNotificationLayer *>(event.layers));

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

        case BGPEventList::ConnectRetryTimer_Expires:
        case BGPEventList::DelayOpenTimer_Expires:
        case BGPEventList::IdleHoldTimer_Expires:
        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventList::BGPHeaderErr:
        case BGPEventList::BGPOpenMsgErr:
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
