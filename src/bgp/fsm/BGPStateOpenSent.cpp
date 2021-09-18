#include "BGPStateOpenSent.h"

#include "../../entities/Router.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPKeepaliveLayer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPNotificationLayer.h"
#include "../packets/BGPOpenLayer.h"
#include "BGPStateActive.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"
#include "BGPStateOpenConfirm.h"


bool BGPStateOpenSent ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventList) {
        case BGPEventList::ManualStart:
        case BGPEventList::AutomaticStart:
        case BGPEventList::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventList::AutomaticStart_with_PassiveTcpEstablishment:
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
            // sends the NOTIFICATION with a Cease,

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer =
                    std::make_unique<BGPNotificationLayer>(
                        BGPNotificationLayer::CEASE,
                        BGPNotificationLayer::ERR_X_NO_SUB_ERR);
                bgpNotificationLayer->computeCalculateFields();

                // Commented code below is for tests
                // XXX change it to test notification
                /* BGPOpenLayer bgpOpenMessage =
                    BGPOpenLayer(stateMachine->connection->owner->AS_number,
                                 stateMachine->getHoldTime().count(),
                                 stateMachine->connection->bgpApplication
                                     ->getBGPIdentifier());
                std::cout << bgpOpenMessage.toString() << std::endl; */

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero
            stateMachine->setConnectRetryCounter(0);

            // and changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::AutomaticStop:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> AutomaticStop");
            // sends the NOTIFICATION with a Cease,

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
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all the BGP resources, done

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

        case BGPEventList::HoldTimer_Expires:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> HoldTimer_Expires");
            // sends a NOTIFICATION message with the error code Hold Timer
            // Expired

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
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter,
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

        case BGPEventList::TcpConnection_Valid:
        case BGPEventList::Tcp_CR_Acked:
        case BGPEventList::TcpConnectionConfirmed:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> TcpConnection_Valid, Tcp_CR_Acked, "
                    "TcpConnectionConfirmed");
            // MANDATORY -- How?
            // TODO a second TCP connection may be in progress. This second TCP
            // connection is tracked per Connection Collision processing
            // (Section 6.8) until an OPEN message is received.

            handled = false;
            break;

        case BGPEventList::Tcp_CR_Invalid:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> Tcp_CR_Invalid");
            // A TCP Connection Request for an Invalid port (Tcp_CR_Invalid
            // (Event 15)) is ignored.
            break;

        case BGPEventList::TcpConnectionFails:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> TcpConnectionFails");
            // closes the BGP connection,

            // FIXME Check if it is ok, because by dropping the connection also
            // the state machine will be deallocated :)
            stateMachine->connection->closeConnection();

            // restarts the ConnectRetryTimer,
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // continues to listen for a connection that may be initiated
            // --> Nothing to do as the listening is already taking place.

            // changes its state to Active.
            stateMachine->changeState(new BGPStateActive(stateMachine));

            break;

        case BGPEventList::BGPOpen:
            L_DEBUG(stateMachine->connection->owner->ID, "Event -> BGPOpen");

            // When an OPEN message is received, all fields are checked for
            // correctness.  If there are no errors in the OPEN message (Event
            // 19), the local system:

            // TODO Collision detection mechanisms (Section 6.8) need to be
            // applied
            // when a valid BGP OPEN message is received (Event 19 or Event 20).

            // FIXME check how to call collision detection
            // stateMachine->connection->bgpApplication->collisionDetection(stateMachine->connection);


            // resets the DelayOpenTimer to zero,
            stateMachine->resetDelayOpenTimer();

            // sets the BGP ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // sends a KEEPALIVE message, and
            // TODO test KEEPALIVE message
            {
                std::unique_ptr<BGPLayer> bgpKeepaliveLayer =
                    std::make_unique<BGPKeepaliveLayer>();
                bgpKeepaliveLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpKeepaliveLayer));

                stateMachine->connection->sendData(std::move(layers));
            }

            // sets a KeepaliveTimer (via the text below)

            // sets the HoldTimer according to the negotiated value (see
            // Section 4.2),
            {
                BGPOpenLayer *openLayer =
                    dynamic_cast<BGPOpenLayer *>(event.layers);

                BGPOpenLayer::BGPOpenHeader *openHeader =
                    openLayer->getOpenHeaderOrNull();

                if (openHeader->holdTime_be <
                    stateMachine->getNegotiatedHoldTime().count()) {
                    stateMachine->setNegotiatedKeepaliveTime(
                        std::chrono::seconds(openHeader->holdTime_be / 3));
                    stateMachine->setNegotiatedHoldTime(
                        std::chrono::seconds(openHeader->holdTime_be));
                }
            }

            // If the negotiated hold time value is zero, then the HoldTimer and
            // KeepaliveTimer are not started.

            if (stateMachine->getNegotiatedHoldTime() != 0s) {
                stateMachine->resetHoldTimer();
                stateMachine->resetKeepAliveTimer();
                stateMachine->holdTimer->start();
                stateMachine->keepAliveTimer->start();
            }

            // changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenConfirm(stateMachine));

            // TODO If the value of the Autonomous
            // System field is the same as the local Autonomous System number,
            // then the connection is an "internal" connection; otherwise, it is
            // an "external" connection.

            break;

        case BGPEventList::BGPHeaderErr:
        case BGPEventList::BGPOpenMsgErr:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> BGPHeaderErr, BGPOpenMsgErr");
            //  sends a NOTIFICATION message with the appropriate error
            //  code

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer(
                    dynamic_cast<BGPNotificationLayer *>(event.layers));

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpNotificationLayer));

                stateMachine->connection->sendData(std::move(layers));
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources,

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

        case BGPEventList::OpenCollisionDump:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> OpenCollisionDump");
            // OPTIONAL
            // TODO remove if not needed
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

        case BGPEventList::NotifMsgVerErr:
            L_DEBUG(stateMachine->connection->owner->ID,
                    "Event -> NotifMsgVerErr");
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::ConnectRetryTimer_Expires:
        case BGPEventList::KeepaliveTimer_Expires:
        case BGPEventList::DelayOpenTimer_Expires:
        case BGPEventList::IdleHoldTimer_Expires:
        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventList::NotifMsg:
        case BGPEventList::KeepAliveMsg:
        case BGPEventList::UpdateMsg:
        case BGPEventList::UpdateMsgErr:
            L_DEBUG(
                stateMachine->connection->owner->ID,
                "Event -> ManualConnectRetryTimer_Expires, "
                "KeepaliveTimer_Expires, DelayOpenTimer_Expires, "
                "IdleHoldTimer_Expires, BGPOpen_with_DelayOpenTimer_running, "
                "NotifMsg, KeepAliveMsg, UpdateMsg, UpdateMsgErrStop");
            // sends the NOTIFICATION with the Error Code Finite State
            // Machine Error

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
            }

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources,

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
//  If the negotiated hold time value is zero, then the HoldTimer and
//       KeepaliveTimer are not started.  If the value of the Autonomous
//       System field is the same as the local Autonomous System number,
//       then the connection is an "internal" connection; otherwise, it is
//       an "external" connection.  (This will impact UPDATE processing as
//       described below.)
