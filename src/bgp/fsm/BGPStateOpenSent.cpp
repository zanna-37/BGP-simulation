#include "BGPStateOpenSent.h"

#include "BGPStateActive.h"
#include "BGPStateIdle.h"
#include "BGPStateOpenConfirm.h"

BGPStateOpenSent ::~BGPStateOpenSent() {}

bool BGPStateOpenSent ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event) {
        case BGPEvent::ManualStop:
            // TODO sends the NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

            // - sets the ConnectRetryCounter to zero, and
            stateMachine->setConnectRetryCounter(0);

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::AutomaticStop:
            // TODO sends the NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all the BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //     DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::HoldTimer_Expires:
            // TODO sends a NOTIFICATION message with the error code Hold Timer
            //   Expired,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

            // - increments the ConnectRetryCounter,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }


            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::TcpConnection_Valid:
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:
            //    TODO a second TCP
            //   connection may be in progress.  This second TCP connection is
            //   tracked per Connection Collision processing (Section 6.8) until
            //   an OPEN message is received.
            break;
        case BGPEvent::Tcp_CR_Invalid:
            break;
        case BGPEvent::TcpConnectionFails:
            // TODO closes the BGP connection,
            stateMachine->connection->tcpConnection = nullptr;

            // - restarts the ConnectRetryTimer,
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // TODO continues to listen for a connection that may be initiated
            // by
            //   the remote BGP peer, and

            // - changes its state to Active.
            stateMachine->changeState(new BGPStateActive(stateMachine));
            break;
        case BGPEvent::BGPOpen:
            // - resets the DelayOpenTimer to zero,
            stateMachine->resetDelayOpenTimer();

            // TODO sets the BGP ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO sends a KEEPALIVE message, and

            // TODO sets a KeepaliveTimer (via the text below)

            // TODO sets the HoldTimer according to the negotiated value (see
            //   Section 4.2),
            stateMachine->resetHoldTimer();
            stateMachine->holdTimer->setDuration(
                stateMachine->connection->holdTime);
            stateMachine->holdTimer->start();

            // - changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenConfirm(stateMachine));
            break;
        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:
            //  TODO sends a NOTIFICATION message with the appropriate error
            //  code,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::OpenCollisionDump:
            // TODO sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            break;
        case BGPEvent::NotifMsgVerErr:
            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection, and
            dropTCPConnection();

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::ConnectRetryTimer_Expires:
        case BGPEvent::KeepaliveTimer_Expires:
        case BGPEvent::DelayOpenTimer_Expires:
        case BGPEvent::IdleHoldTimer_Expires:
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
        case BGPEvent::NotifMsg:
        case BGPEvent::KeepAliveMsg:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
            // TODO sends the NOTIFICATION with the Error Code Finite State
            //   Machine Error,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,
            dropTCPConnection();

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
//  If the negotiated hold time value is zero, then the HoldTimer and
//       KeepaliveTimer are not started.  If the value of the Autonomous
//       System field is the same as the local Autonomous System number,
//       then the connection is an "internal" connection; otherwise, it is
//       an "external" connection.  (This will impact UPDATE processing as
//       described below.)