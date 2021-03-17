#include "BGPStateActive.h"

BGPStateActive :: ~BGPStateActive(){

}

void BGPStateActive :: enter(){

}
void BGPStateActive :: execute(){

}

void BGPStateActive :: exit(){

}

bool BGPStateActive :: OnEvent(Event event){

    bool handled = true;
    switch (event)
    {
    case ManualStop:
        // - If the DelayOpenTimer is running and the
        //   SendNOTIFICATIONwithoutOPEN session attribute is set, the
        //   local system sends a NOTIFICATION with a Cease,

        // - releases all BGP resources including stopping the
        //   DelayOpenTimer

        // - drops the TCP connection,

        // - sets ConnectRetryCounter to zero,

        // - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
        //   zero, and

        // - changes its state to Idle.
        break;
    case ConnectRetryTimer_Expires:
        //  - restarts the ConnectRetryTimer (with initial value),

        // - initiates a TCP connection to the other BGP peer,

        // - continues to listen for a TCP connection that may be initiated
        //   by a remote BGP peer, and

        // - changes its state to Connect.
        break;
    case DelayOpenTimer_Expires:
        //  - sets the ConnectRetryTimer to zero,

        // - stops and clears the DelayOpenTimer (set to zero),

        // - completes the BGP initialization,

        // - sends the OPEN message to its remote peer,

        // - sets its hold timer to a large value ( A HoldTimer value of 4 minutes is also suggested for this state transition.), and

        // - changes its state to OpenSent.

        break;
    case TcpConnection_Valid:
        // the local system processes the TCP connection flags and stays
        // in the Active state.
        break;
    case Tcp_CR_Invalid:
        // the local system rejects the TCP connection and stays in the
        // Active State.
        break;
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:

        // If the DelayOpen attribute is set to TRUE, the local system:

        //   - stops the ConnectRetryTimer and sets the ConnectRetryTimer
        //     to zero,

        //   - sets the DelayOpenTimer to the initial value
        //     (DelayOpenTime), and

        //   - stays in the Active state.

        // If the DelayOpen attribute is set to FALSE, the local system:

        //   - sets the ConnectRetryTimer to zero,

        //   - completes the BGP initialization,

        //   - sends the OPEN message to its peer,

        //   - sets its HoldTimer to a large value, and

        //   - changes its state to OpenSent.
        break;
    case TcpConnectionFails:
        // - restarts the ConnectRetryTimer (with the initial value),

        // - stops and clears the DelayOpenTimer (sets the value to zero),

        // - releases all BGP resource,

        // - increments the ConnectRetryCounter by 1,

        // - optionally performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case BGPOpen_with_DelayOpenTimer_running:
        //  - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - stops and clears the DelayOpenTimer (sets to zero),

        // - completes the BGP initialization,

        // - sends an OPEN message,

        // - sends a KEEPALIVE message,

        // - if the HoldTimer value is non-zero,

        //     - starts the KeepaliveTimer to initial value,

        //     - resets the HoldTimer to the negotiated value,

        //   else if the HoldTimer is zero

        //     - resets the KeepaliveTimer (set to zero),

        //     - resets the HoldTimer to zero, and

        // - changes its state to OpenConfirm.
        break;
    case BGPOpenMsgErr:
    case BGPHeaderErr:
        //  - (optionally) sends a NOTIFICATION message with the appropriate
        //   error code if the SendNOTIFICATIONwithoutOPEN attribute is set
        //   to TRUE,

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case NotifMsgVerErr:
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - stops and resets the DelayOpenTimer (sets to zero),

        // - releases all BGP resources,

        // - drops the TCP connection, and

        // - changes its state to Idle.
        break;
    case AutomaticStop:
    case HoldTimer_Expires:
    case KeepaliveTimer_Expires:
    case BGPOpen:
    case OpenCollisionDump:
    case NotifMsg:
    case KeepAliveMsg:
    case UpdateMsg:
    case UpdateMsgErr:

        // - sets the ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by one,

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
// If the DelayOpenTimer is not running, the local system:

//         - sets the ConnectRetryTimer to zero,

//         - releases all BGP resources,

//         - drops the TCP connection,

//         - increments the ConnectRetryCounter by 1,

//         - (optionally) performs peer oscillation damping if the
//           DampPeerOscillations attribute is set to TRUE, and

//         - changes its state to Idle.