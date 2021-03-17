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
bool BGPStateConnect :: OnEvent(Event event) { 

   bool handled = true;

    switch (event){
    case ManualStop:
        // - drops the TCP connection,

        // - releases all BGP resources,

        stateMachine->setConnectRetryCounter(0);
        // - stops the ConnectRetryTimer and sets ConnectRetryTimer to zero, and

        stateMachine->ChangeState(new BGPStateIdle(stateMachine));
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

        stateMachine->ChangeState(new BGPStateOpenSent(stateMachine));
        break;
    case TcpConnection_Valid:
        // the TCP connection is processed,
        break;
    case Tcp_CR_Invalid:

        // the local system rejects the TCP connection
        break;
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:
        if(stateMachine->delayOpen()){
        //     - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        //     - sets the DelayOpenTimer to the initial value, and

        }else{
        //     - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        //     - completes BGP initialization

        //     - sends an OPEN message to its peer,

        //     - sets the HoldTimer to a large value (A HoldTimer value of 4 minutes is suggested.), and
        }
        stateMachine->ChangeState(new BGPStateOpenSent(stateMachine));
        
        break;

    case TcpConnectionFails:
        // - restarts the ConnectRetryTimer with the initial value,

        // - stops the DelayOpenTimer and resets its value to zero,

        // - continues to listen for a connection that may be initiated by
        //   the remote BGP peer, and

        stateMachine->ChangeState(new BGPStateActive(stateMachine));
        break;
    case BGPOpen_with_DelayOpenTimer_running:
        //  - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - completes the BGP initialization,
        // - stops and clears the DelayOpenTimer (sets the value to zero),

        // - sends an OPEN message,

        // - sends a KEEPALIVE message,

        if(stateMachine->holdTime() != 0){
            // - starts the KeepaliveTimer with the initial value and

            // - resets the HoldTimer to the negotiated value,
        }else{
            // - resets the KeepaliveTimer and

            // - resets the HoldTimer value to zero,
        }

        stateMachine->ChangeState(new BGPStateOpenConfirm(stateMachine));

        break;
    case BGPHeaderErr:
    case BGPOpenMsgErr:

        if(stateMachine->sendNOTIFICATIONwithoutOPEN()){
        //   the local system first sends a NOTIFICATION
        //   message with the appropriate error code, and then
        }

        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,
        stateMachine->incrementConnectRetryCounter();

        if(stateMachine->dampPeerOscillations()){
            // - (optionally) performs peer oscillation 
        }

        // - changes its state to Idle.
        stateMachine->ChangeState(new BGPStateIdle(stateMachine));

        break;
    case NotifMsgVerErr:
        // If the DelayOpenTimer is running, the local system:
        if(true){
            // - stops the ConnectRetryTimer (if running) and sets the
            //   ConnectRetryTimer to zero,

            // - stops and resets the DelayOpenTimer (sets to zero),

            // - releases all BGP resources,

            // - drops the TCP connection, and

            // - changes its state to Idle.
        }else{
        // If the DelayOpenTimer is not running, the local system:

        // - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
        //   zero,

        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,
        stateMachine->incrementConnectRetryCounter();

        if(stateMachine->dampPeerOscillations()){
        // - performs peer oscillation damping if the DampPeerOscillations
        //   attribute is set to True, and
        }
        }
        stateMachine->ChangeState(new BGPStateIdle(stateMachine));

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
        // - if the ConnectRetryTimer is running, 
        if(true){
        //   stops and resets the ConnectRetryTimer (sets to zero),
        }

        // - if the DelayOpenTimer is running, stops and resets the
        if(true){
        //   DelayOpenTimer (sets to zero),
        }
        // - releases all BGP resources,

        // - drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,
        stateMachine->incrementConnectRetryCounter();

        if(stateMachine->dampPeerOscillations()){
        // - performs peer oscillation damping if the DampPeerOscillations
        //   attribute is set to True, and
        }
        // - changes its state to Idle.
        stateMachine->ChangeState(new BGPStateIdle(stateMachine));

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