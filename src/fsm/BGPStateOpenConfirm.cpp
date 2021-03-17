#include "BGPStateOpenConfirm.h"

bool BGPStateOpenConfirm :: OnEvent(Event event){

    bool handled = true;

    switch (event)
    {
    case ManualStop:
        // - sends the NOTIFICATION message with a Cease,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - sets the ConnectRetryCounter to zero,

        // - sets the ConnectRetryTimer to zero, and

        // - changes its state to Idle.
        break;
    case AutomaticStop:
        // - sends the NOTIFICATION message with a Cease,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case HoldTimer_Expires:
        // - sends the NOTIFICATION message with the Error Code Hold Timer
        //   Expired,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case KeepaliveTimer_Expires:

        // - sends a KEEPALIVE message,

        // - restarts the KeepaliveTimer, and

        // - remains in the OpenConfirmed state.
        break;
    case TcpConnection_Valid:
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:
        //      the local system needs to track the second
        //   connection.
        break;
    case Tcp_CR_Invalid:
        // the local system will ignore the second connection attempt.
        break;
    case TcpConnectionFails:
    case NotifMsg:
        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case NotifMsgVerErr:
        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection, and

        // - changes its state to Idle.
        break;
    case BGPOpen:
        // If this connection is to be dropped due to connection collision,
        // the local system:
            // - sends a NOTIFICATION with a Cease,

            // - sets the ConnectRetryTimer to zero,

            // - releases all BGP resources,

            // - drops the TCP connection (send TCP FIN),

            // - increments the ConnectRetryCounter by 1,

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and

            // - changes its state to Idle.
        break;
    case BGPHeaderErr:
    case BGPOpenMsgErr:
        // - sends a NOTIFICATION message with the appropriate error code,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case OpenCollisionDump:
        // - sends a NOTIFICATION with a Cease,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case KeepAliveMsg:

        // - restarts the HoldTimer and

        // - changes its state to Established.
        break;
    case ConnectRetryTimer_Expires:
    case DelayOpenTimer_Expires:
    case IdleHoldTimer_Expires:
    case BGPOpen_with_DelayOpenTimer_running:
    case UpdateMsg:
    case UpdateMsgErr:
        // - sends a NOTIFICATION with a code of Finite State Machine
        //   Error,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;

    default:
        handled = false;
        break;
    }
}