#include "BGPStateOpenConfirm.h"

#include "BGPStateEnstablished.h"


BGPStateOpenConfirm ::~BGPStateOpenConfirm() {}

bool BGPStateOpenConfirm ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event) {
        case BGPEvent::ManualStop:
            // TODO sends the NOTIFICATION message with a Cease,

            // TODO releases all BGP resources,

            // TODO drops the TCP connection,

            // - sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - sets the ConnectRetryTimer to zero, and
            stateMachine->resetConnectRetryTimer();

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::AutomaticStop:
            // TODO sends the NOTIFICATION message with a Cease,

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
        case BGPEvent::HoldTimer_Expires:
            // TODO sends the NOTIFICATION message with the Error Code Hold
            // Timer
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
        case BGPEvent::KeepaliveTimer_Expires:

            // TODO sends a KEEPALIVE message,

            // - restarts the KeepaliveTimer, and
            stateMachine->resetKeepAliveTimer();
            stateMachine->keepAliveTimer->start();

            // - remains in the OpenConfirmed state.
            break;
        case BGPEvent::TcpConnection_Valid:
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:
            //   TODO   the local system needs to track the second
            //   connection.
            break;
        case BGPEvent::Tcp_CR_Invalid:
            // TODO the local system will ignore the second connection attempt.
            break;
        case BGPEvent::TcpConnectionFails:
        case BGPEvent::NotifMsg:
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
        case BGPEvent::NotifMsgVerErr:
            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection, and

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::BGPOpen:
            // TODO If this connection is to be dropped due to connection
            // collision, the local system:
            // TODO sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // TODO drops the TCP connection (send TCP FIN),

            // - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;
        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:
            // TODO sends a NOTIFICATION message with the appropriate error
            // code,

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
        case BGPEvent::OpenCollisionDump:
            // TODO sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources

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
        case BGPEvent::KeepAliveMsg:

            // FIXME restarts the HoldTimer and
            stateMachine->resetHoldTimer();
            stateMachine->holdTimer->start();

            // - changes its state to Established.
            stateMachine->changeState(new BGPStateEnstablished(stateMachine));
            break;
        case BGPEvent::ConnectRetryTimer_Expires:
        case BGPEvent::DelayOpenTimer_Expires:
        case BGPEvent::IdleHoldTimer_Expires:
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
            // TODO sends a NOTIFICATION with a code of Finite State Machine
            //   Error,

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