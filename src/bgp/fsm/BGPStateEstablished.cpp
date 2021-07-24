#include "BGPStateEstablished.h"

#include <chrono>

#include "../../entities/Router.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"


bool BGPStateEstablished ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventList) {
        case BGPEventList::ManualStop:
            // TODO sends the NOTIFICATION message with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases BGP resources,

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // sets the ConnectRetryCounter to zero, and
            stateMachine->setConnectRetryCounter(0);

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventList::AutomaticStop:
            // TODO sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

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

        case BGPEventList::HoldTimer_Expires:
            // TODO sends a NOTIFICATION message with the Error Code Hold Timer
            // Expired,

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

        case BGPEventList::KeepaliveTimer_Expires:
            // TODO sends a KEEPALIVE message, and

            // FIXME restarts its KeepaliveTimer, unless the negotiated HoldTime
            // value is zero.
            if (stateMachine->holdTimer->getDuration() != 0ms) {
                stateMachine->resetKeepAliveTimer();
                stateMachine->keepAliveTimer->start();
            }

            handled = false;
            break;

        case BGPEventList::TcpConnection_Valid:
            // TODO received for a valid port, will cause the second connection
            // to be tracked.

            handled = false;
            break;

        case BGPEventList::Tcp_CR_Acked:
        case BGPEventList::TcpConnectionConfirmed:
            // TODO the second connection SHALL be tracked until it sends an
            // OPEN message.

            handled = false;
            break;

        case BGPEventList::BGPOpen:
            // TODO If a valid OPEN message (BGPOpen (Event 19)) is received,
            // and if the CollisionDetectEstablishedState optional attribute is
            // TRUE, the OPEN message will be checked to see if it collides
            // (Section 6.8) with any other connection.
            handled = false;
            break;

        case BGPEventList::OpenCollisionDump:
            // TODO sends a NOTIFICATION with a Cease,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

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

        case BGPEventList::NotifMsgVerErr:
        case BGPEventList::NotifMsg:
        case BGPEventList::TcpConnectionFails:
            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases all the BGP resources,

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEventList::KeepAliveMsg:
            // FIXME restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.

            handled = false;
            break;

        case BGPEventList::UpdateMsg:
            // TODO processes the message,

            // restarts its HoldTimer, if the negotiated HoldTime value is
            // non-zero, and
            if (stateMachine->getHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // remains in the Established state.

            handled = false;
            break;

        case BGPEventList::UpdateMsgErr:
            // TODO sends a NOTIFICATION message with an Update error,

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

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

        case BGPEventList::ConnectRetryTimer_Expires:
        case BGPEventList::DelayOpenTimer_Expires:
        case BGPEventList::IdleHoldTimer_Expires:
        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventList::BGPHeaderErr:
        case BGPEventList::BGPOpenMsgErr:
            // TODO sends a NOTIFICATION message with the Error Code Finite
            // State Machine Error,

            // TODO deletes all routes associated with this connection,

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
