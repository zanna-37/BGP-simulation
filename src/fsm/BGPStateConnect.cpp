#include "BGPStateConnect.h"
#include "BGPStateIdle.h"
#include "BGPStateOpenSent.h"
#include "BGPStateActive.h"
#include "BGPStateOpenConfirm.h"


BGPStateConnect :: ~BGPStateConnect(){

}

void BGPStateConnect :: enter(){

}
void BGPStateConnect :: execute(){

}

void BGPStateConnect :: exit(){

}
bool BGPStateConnect :: onEvent(Event event) { 

   bool handled = true;

    switch (event){
    case ManualStop:
        // - drops the TCP connection,

        // - releases all BGP resources,

        // - sets ConnectRetryCounter to zero,

        // - stops the ConnectRetryTimer and sets ConnectRetryTimer to
        //   zero, and

        // - changes its state to Idle.

        break;
    case ConnectRetryTimer_Expires:
        // - drops the TCP connection,

        // - restarts the ConnectRetryTimer,

        // - stops the DelayOpenTimer and resets the timer to zero,

        // - initiates a TCP connection to the other BGP peer,

        // - continues to listen for a connection that may be initiated by
        //   the remote BGP peer, and

        break;
    case DelayOpenTimer_Expires:
        // - sends an OPEN message to its peer,

        // - sets the HoldTimer to a large value, and

        // - changes its state to OpenSent.
        break;
    case TcpConnection_Valid:
        // the TCP connection is processed,
        break;
    case Tcp_CR_Invalid:

        // the local system rejects the TCP connection
        break;
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:

        // If the DelayOpen attribute is set to TRUE, the local system:
        
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - sets the DelayOpenTimer to the initial value, and

        // - stays in the Connect state.

        // If the DelayOpen attribute is set to FALSE, the local system:

        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - completes BGP initialization

        // - sends an OPEN message to its peer,

        // - sets the HoldTimer to a large value, and

        // - changes its state to OpenSent.
        
        break;

    case TcpConnectionFails:
    //     If the DelayOpenTimer is running, the local
    //   system:

    //     - restarts the ConnectRetryTimer with the initial value,

    //     - stops the DelayOpenTimer and resets its value to zero,

    //     - continues to listen for a connection that may be initiated by
    //       the remote BGP peer, and

    //     - changes its state to Active.

    //   If the DelayOpenTimer is not running, the local system:

    //     - stops the ConnectRetryTimer to zero,

    //     - drops the TCP connection,

    //     - releases all BGP resources, and

    //     - changes its state to Idle.
        break;
    case BGPOpen_with_DelayOpenTimer_running:
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - completes the BGP initialization,

        // - stops and clears the DelayOpenTimer (sets the value to zero),

        // - sends an OPEN message,

        // - sends a KEEPALIVE message,

        // - if the HoldTimer initial value is non-zero,

        //     - starts the KeepaliveTimer with the initial value and

        //     - resets the HoldTimer to the negotiated value,

        //   else, if the HoldTimer initial value is zero,

        //     - resets the KeepaliveTimer and

        //     - resets the HoldTimer value to zero,

        // - and changes its state to OpenConfirm.
        break;
    case BGPHeaderErr:
    case BGPOpenMsgErr:

        // - (optionally) If the SendNOTIFICATIONwithoutOPEN attribute is
        //   set to TRUE, then the local system first sends a NOTIFICATION
        //   message with the appropriate error code, and then

        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.

        break;
    case NotifMsgVerErr:
    //     - stops the ConnectRetryTimer (if running) and sets the
    //       ConnectRetryTimer to zero,

    //     - stops and resets the DelayOpenTimer (sets to zero),

    //     - releases all BGP resources,

    //     - drops the TCP connection, and

    //     - changes its state to Idle.

    //   If the DelayOpenTimer is not running, the local system:

    //     - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
    //       zero,

    //     - releases all BGP resources,

    //     - drops the TCP connection,

    //     - increments the ConnectRetryCounter by 1,

    //     - performs peer oscillation damping if the DampPeerOscillations
    //       attribute is set to True, and

    //     - changes its state to Idle.
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
        // - if the ConnectRetryTimer is running, stops and resets the
        //   ConnectRetryTimer (sets to zero),

        // - if the DelayOpenTimer is running, stops and resets the
        //   DelayOpenTimer (sets to zero),

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - performs peer oscillation damping if the DampPeerOscillations
        //   attribute is set to True, and

        // - changes its state to Idle.

        break;
    default:
        handled = false;
        break;
    }
    return handled;
}

// TODO: check how to implement this
// If the DelayOpenTimer is not running, the local system:

// - stops the ConnectRetryTimer to zero,

// - drops the TCP connection,

// - releases all BGP resources, and

// - changes its state to Idle.

// TODO

    //   If the value of the autonomous system field is the same as the
    //   local Autonomous System number, set the connection status to an
    //   internal connection; otherwise it will be "external".