#include "BGPStateOpenSent.h"

BGPStateOpenSent :: ~BGPStateOpenSent(){

}

void BGPStateOpenSent :: enter(){

}
void BGPStateOpenSent :: execute(){

}

void BGPStateOpenSent :: exit(){

}

bool BGPStateOpenSent :: onEvent(Event event){

    bool handled = true;

    switch (event)
    {
    case ManualStop:
        // - sends the NOTIFICATION with a Cease,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - sets the ConnectRetryCounter to zero, and

        // - changes its state to Idle.
        break;
    case AutomaticStop:
    // - sends the NOTIFICATION with a Cease,

    // - sets the ConnectRetryTimer to zero,

    // - releases all the BGP resources,

    // - drops the TCP connection,

    // - increments the ConnectRetryCounter by 1,

    // - (optionally) performs peer oscillation damping if the
    //     DampPeerOscillations attribute is set to TRUE, and

    // - changes its state to Idle.
        break;
    case HoldTimer_Expires:
        // - sends a NOTIFICATION message with the error code Hold Timer
        //   Expired,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case TcpConnection_Valid:
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:
    //     a second TCP
    //   connection may be in progress.  This second TCP connection is
    //   tracked per Connection Collision processing (Section 6.8) until an
    //   OPEN message is received.
        break;
    case Tcp_CR_Invalid:
        break;
    case TcpConnectionFails:
        // - closes the BGP connection,

        // - restarts the ConnectRetryTimer,

        // - continues to listen for a connection that may be initiated by
        //   the remote BGP peer, and

        // - changes its state to Active.
        break;
    case BGPOpen:
        // - resets the DelayOpenTimer to zero,

        // - sets the BGP ConnectRetryTimer to zero,

        // - sends a KEEPALIVE message, and

        // - sets a KeepaliveTimer (via the text below)

        // - sets the HoldTimer according to the negotiated value (see
        //   Section 4.2),

        // - changes its state to OpenConfirm.
        break;
    case BGPHeaderErr:
    case BGPOpenMsgErr:
        //  sends a NOTIFICATION message with the appropriate error code,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is TRUE, and

        // - changes its state to Idle.
        break;
    case OpenCollisionDump:
        // - sends a NOTIFICATION with a Cease,

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
    case ConnectRetryTimer_Expires:
    case KeepaliveTimer_Expires:
    case DelayOpenTimer_Expires:
    case IdleHoldTimer_Expires:
    case BGPOpen_with_DelayOpenTimer_running:
    case NotifMsg:
    case KeepAliveMsg:
    case UpdateMsg:
    case UpdateMsgErr:
        // - sends the NOTIFICATION with the Error Code Finite State
        //   Machine Error,

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

    return handled;
}

//TODO
//  If the negotiated hold time value is zero, then the HoldTimer and
//       KeepaliveTimer are not started.  If the value of the Autonomous
//       System field is the same as the local Autonomous System number,
//       then the connection is an "internal" connection; otherwise, it is
//       an "external" connection.  (This will impact UPDATE processing as
//       described below.)