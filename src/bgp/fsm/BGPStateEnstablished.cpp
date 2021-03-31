#include "BGPStateEnstablished.h"


BGPStateEnstablished ::~BGPStateEnstablished() {}


bool BGPStateEnstablished ::onEvent(Event event) {
    bool handled = true;

    switch (event) {
        case ManualStop:

            // TODO sends the NOTIFICATION message with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases BGP resources,

            // TODO drops the TCP connection,

            // - sets the ConnectRetryCounter to zero, and
            stateMachine->setConnectRetryCounter(0);

            //  - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case AutomaticStop:
            // TODO sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();
            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case HoldTimer_Expires:
            // TODO sends a NOTIFICATION message with the Error Code Hold Timer
            //   Expired,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case KeepaliveTimer_Expires:
            // TODO sends a KEEPALIVE message, and

            // FIXME restarts its KeepaliveTimer, unless the negotiated HoldTime
            //   value is zero.
            if (stateMachine->holdTimer->getDuration() != 0ms) {
                stateMachine->resetKeepAliveTimer();
                stateMachine->keepAliveTimer->start();
            }
            break;
        case TcpConnection_Valid:
            //   TODO  received for a valid port, will
            //   cause the second connection to be tracked.
            break;
        case Tcp_CR_Acked:
        case TcpConnectionConfirmed:
            //   TODO  the second
            //   connection SHALL be tracked until it sends an OPEN message.
            break;
        case BGPOpen:
            //   TODO    If a valid OPEN message (BGPOpen (Event 19)) is
            //   received, and if the CollisionDetectEstablishedState optional
            //   attribute is TRUE, the OPEN message will be checked to see if
            //   it collides (Section 6.8) with any other connection.
            break;
        case OpenCollisionDump:
            // TODO sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case NotifMsgVerErr:
        case NotifMsg:
        case TcpConnectionFails:
            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases all the BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();
            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case KeepAliveMsg:
            // FIXME restarts its HoldTimer, if the negotiated HoldTime value is
            //   non-zero, and
            if (stateMachine->getHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }


            // - remains in the Established state.
            break;
        case UpdateMsg:
            // TODO processes the message,

            // - restarts its HoldTimer, if the negotiated HoldTime value is
            //   non-zero, and
            if (stateMachine->getHoldTime() != 0ms) {
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->start();
            }

            // - remains in the Established state.
            break;
        case UpdateMsgErr:
            // TODO sends a NOTIFICATION message with an Update error,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO deletes all routes associated with this connection,

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case ConnectRetryTimer_Expires:
        case DelayOpenTimer_Expires:
        case IdleHoldTimer_Expires:
        case BGPOpen_with_DelayOpenTimer_running:
        case BGPHeaderErr:
        case BGPOpenMsgErr:
            // TODO sends a NOTIFICATION message with the Error Code Finite
            // State
            //   Machine Error,

            // TODO deletes all routes associated with this connection,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
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