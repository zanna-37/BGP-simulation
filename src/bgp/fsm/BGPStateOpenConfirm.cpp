#include "BGPStateOpenConfirm.h"

#include "../../entities/Router.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPKeepaliveLayer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPNotificationLayer.h"
#include "BGPStateEstablished.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"


bool BGPStateOpenConfirm ::onEvent(BGPEvent event) {
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
            break;
        case BGPEventList::ManualStop:
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
            }

            // XXX releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // sets the ConnectRetryTimer to zero, and
            stateMachine->resetConnectRetryTimer();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::AutomaticStop:
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

            handled = false;
            break;

        case BGPEventList::HoldTimer_Expires:
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
            }

            // restarts the KeepaliveTimer
            stateMachine->resetKeepAliveTimer();
            stateMachine->keepAliveTimer->start();

            // and remains in the OpenConfirmed state.
            break;

        case BGPEventList::TcpConnection_Valid:
        case BGPEventList::Tcp_CR_Acked:
        case BGPEventList::TcpConnectionConfirmed:
            // TODO MANDATORY TO BE DONE! but How?
            // TODO the local system needs to track the second connection.

            handled = false;
            break;

        case BGPEventList::Tcp_CR_Invalid:
            // OPTIONAL
            // XXX the local system will ignore the second connection attempt.

            handled = false;
            break;

        case BGPEventList::TcpConnectionFails:
        case BGPEventList::NotifMsg:
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
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::BGPOpen:
            // TODO MANDATORY TO BE DONE!
            // TODO If this connection is to be dropped due to connection
            // collision, the local system:

            // And if it does not need to be dropped? The message is ignored??

            // FIXME maybe the collision detection should work in some other way
            // stateMachine->connection->bgpApplication->collisionDetection(stateMachine->connection);


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

            // XXX releases all BGP resources,

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

            handled = false;
            break;

        case BGPEventList::BGPHeaderErr:
        case BGPEventList::BGPOpenMsgErr:
            // MANDATORY
            // sends a NOTIFICATION message with the appropriate error code

            {
                std::unique_ptr<BGPLayer> bgpNotificationLayer(
                        dynamic_cast<BGPNotificationLayer*>(event.layers));

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
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventList::OpenCollisionDump:
            // OPTIONAL
            /* // XXX sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources

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
            stateMachine->changeState(new BGPStateIdle(stateMachine)); */

            handled = false;
            break;

        case BGPEventList::KeepAliveMsg:
            // TODO check if it works
            // FIXME restarts the HoldTimer and
            stateMachine->resetHoldTimer();
            stateMachine->holdTimer->start();

            // changes its state to Established.
            stateMachine->changeState(new BGPStateEstablished(stateMachine));
            break;

        case BGPEventList::ConnectRetryTimer_Expires:
        case BGPEventList::DelayOpenTimer_Expires:
        case BGPEventList::IdleHoldTimer_Expires:
        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventList::UpdateMsg:
        case BGPEventList::UpdateMsgErr:
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

        default:
            handled = false;
            break;
    }
    return handled;
}
