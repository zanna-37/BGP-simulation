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

    switch (event) {
        case BGPEvent::ManualStart:
        case BGPEvent::AutomaticStart:
        case BGPEvent::ManualStart_with_PassiveTcpEstablishment:
        case BGPEvent::AutomaticStart_with_PassiveTcpEstablishment:
        case BGPEvent::AutomaticStart_with_DampPeerOscillations:
        case BGPEvent::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            // (Events 1, 3-7) are ignored in the Active state.
            break;
        case BGPEvent::ManualStop:
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

            // TODO releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // sets the ConnectRetryTimer to zero, and
            stateMachine->resetConnectRetryTimer();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEvent::AutomaticStop:
            // OPTIONAL
            // TODO remove if not necessary
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

            // TODO releases all BGP resources, done

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

        case BGPEvent::HoldTimer_Expires:
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

            // TODO releases all BGP resources, done

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

        case BGPEvent::KeepaliveTimer_Expires:
            // TODO sends a KEEPALIVE message,

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

        case BGPEvent::TcpConnection_Valid:
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:
            // TODO MANDATORY TO BE DONE!
            // TODO the local system needs to track the second connection. 

            handled = false;
            break;

        case BGPEvent::Tcp_CR_Invalid:
        // OPTIONAL
            // TODO the local system will ignore the second connection attempt.

            handled = false;
            break;

        case BGPEvent::TcpConnectionFails:
        case BGPEvent::NotifMsg:
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources, done

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

        case BGPEvent::NotifMsgVerErr:
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEvent::BGPOpen:
            // TODO MANDATORY TO BE DONE!s
            // TODO If this connection is to be dropped due to connection
            // collision, the local system:
            // TODO sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

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

        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:
        // TODO MANDATORY
            // TODO sends a NOTIFICATION message with the appropriate error code

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

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

        case BGPEvent::OpenCollisionDump:
            // OPTIONAL
            /* // TODO sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources

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

        case BGPEvent::KeepAliveMsg:
            // TODO check if it works
            // FIXME restarts the HoldTimer and 
            stateMachine->resetHoldTimer();
            stateMachine->holdTimer->start();

            // changes its state to Established.
            stateMachine->changeState(new BGPStateEstablished(stateMachine));
            break;

        case BGPEvent::ConnectRetryTimer_Expires:
        case BGPEvent::DelayOpenTimer_Expires:
        case BGPEvent::IdleHoldTimer_Expires:
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
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

            // TODO releases all BGP resources, done

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
