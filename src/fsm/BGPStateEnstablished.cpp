#include "BGPStateEnstablished.h"


BGPStateEnstablished :: ~BGPStateEnstablished(){

}

void BGPStateEnstablished :: enter(){

}
void BGPStateEnstablished :: execute(){

}

void BGPStateEnstablished :: exit(){

}


bool BGPStateEnstablished :: OnEvent(Event event){

    bool handled = true;

    switch (event)
    {
    case ManualStop:

        // - sends the NOTIFICATION message with a Cease,

        // - sets the ConnectRetryTimer to zero,

        // - deletes all routes associated with this connection,

        // - releases BGP resources,

        // - drops the TCP connection,

        // - sets the ConnectRetryCounter to zero, and

        //  - changes its state to Idle.        
        break;
    case AutomaticStop:
        // - sends a NOTIFICATION with a Cease,

        // - sets the ConnectRetryTimer to zero

        // - deletes all routes associated with this connection,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case HoldTimer_Expires:
        // - sends a NOTIFICATION message with the Error Code Hold Timer
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
        // - sends a KEEPALIVE message, and

        // - restarts its KeepaliveTimer, unless the negotiated HoldTime
        //   value is zero.
        break;
    case TcpConnection_Valid:
        //     received for a valid port, will
        //   cause the second connection to be tracked.
        break;
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:
        //     the second
        //   connection SHALL be tracked until it sends an OPEN message.
        break;
    case BGPOpen:
        //       If a valid OPEN message (BGPOpen (Event 19)) is received, and if
        //   the CollisionDetectEstablishedState optional attribute is TRUE,
        //   the OPEN message will be checked to see if it collides (Section
        //   6.8) with any other connection. 
        break;
    case OpenCollisionDump:
        // - sends a NOTIFICATION with a Cease,

        // - sets the ConnectRetryTimer to zero,

        // - deletes all routes associated with this connection,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations is set to TRUE, and

        // - changes its state to Idle.
        break;
    case NotifMsgVerErr:
    case NotifMsg:
    case TcpConnectionFails:
        // - sets the ConnectRetryTimer to zero,

        // - deletes all routes associated with this connection,

        // - releases all the BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - changes its state to Idle.
        break;
    case KeepAliveMsg:
        // - restarts its HoldTimer, if the negotiated HoldTime value is
        //   non-zero, and

        // - remains in the Established state.
        break;
    case UpdateMsg:
        // - processes the message,

        // - restarts its HoldTimer, if the negotiated HoldTime value is
        //   non-zero, and

        // - remains in the Established state.
        break;
    case UpdateMsgErr:
        // - sends a NOTIFICATION message with an Update error,

        // - sets the ConnectRetryTimer to zero,

        // - deletes all routes associated with this connection,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,

        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and

        // - changes its state to Idle.
        break;
    case ConnectRetryTimer_Expires:
    case DelayOpenTimer_Expires:
    case IdleHoldTimer_Expires:
    case BGPOpen_with_DelayOpenTimer_running:
    case BGPHeaderErr:
    case BGPOpenMsgErr:
        // - sends a NOTIFICATION message with the Error Code Finite State
        //   Machine Error,

        // - deletes all routes associated with this connection,

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
    //   Each time the local system sends a KEEPALIVE or UPDATE message, it
    //   restarts its KeepaliveTimer, unless the negotiated HoldTime value
    //   is zero.