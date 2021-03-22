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
        // TODO drops the TCP connection,

        // TODO releases all BGP resources,

        // - sets ConnectRetryCounter to zero,
        stateMachine->setConnectRetryCounter(0);

        // - stops the ConnectRetryTimer and sets ConnectRetryTimer to
        //   zero, and
        stateMachine->connectRetryTimer->reset();

        // - changes its state to Idle.
        stateMachine->changeState(new BGPStateIdle(stateMachine));

        break;
    case ConnectRetryTimer_Expires:
        // TODO drops the TCP connection,

        // - restarts the ConnectRetryTimer,
        stateMachine->connectRetryTimer->start();

        // - stops the DelayOpenTimer and resets the timer to zero,
        stateMachine->delayOpenTimer->reset();

        // TODO initiates a TCP connection to the other BGP peer,

        // TODO continues to listen for a connection that may be initiated by
        //   the remote BGP peer

        break;
    case DelayOpenTimer_Expires:
        // TODO sends an OPEN message to its peer,

        // - NOT_SURE sets the HoldTimer to a large value, and
        stateMachine->setHoldTime(240s); //4 minutes suggested
        stateMachine->holdTimer->start(); //do we need to start it?

        // - changes its state to OpenSent.
        stateMachine->changeState(new BGPStateOpenSent(stateMachine));
        break;
    case TcpConnection_Valid:
        // TODO the TCP connection is processed,
        break;
    case Tcp_CR_Invalid:

        // TODO the local system rejects the TCP connection
        break;
    case Tcp_CR_Acked:
    case TcpConnectionConfirmed:

        // If the DelayOpen attribute is set to TRUE, the local system:
        if(stateMachine->getDelayOpen()){
        
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,
        stateMachine->connectRetryTimer->reset();

        // - sets the DelayOpenTimer to the initial value, and
        stateMachine->delayOpenTimer->start();

        // - stays in the Connect state.

        // If the DelayOpen attribute is set to FALSE, the local system:
        }else{
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,
        stateMachine->connectRetryTimer->reset();

        // TODO completes BGP initialization

        // TODO sends an OPEN message to its peer,

        // - NOT_SURE sets the HoldTimer to a large value, and
        stateMachine->setDelayOpenTime(240s);
        stateMachine->delayOpenTimer->start();

        // - changes its state to OpenSent.
        stateMachine->changeState(new BGPStateOpenSent(stateMachine));
        
        }
        break;

    case TcpConnectionFails:
    //     If the DelayOpenTimer is running, the local
    //   system:
    if(stateMachine->delayOpenTimer->isRunning()){


    //     - restarts the ConnectRetryTimer with the initial value,
        stateMachine->connectRetryTimer->stop();
        stateMachine->connectRetryTimer->start();

    //     - stops the DelayOpenTimer and resets its value to zero,
        stateMachine->delayOpenTimer->reset();

    //     -  TODO continues to listen for a connection that may be initiated by
    //       the remote BGP peer, and

    //     - changes its state to Active.
        stateMachine->changeState(new BGPStateActive(stateMachine));

    //   If the DelayOpenTimer is not running, the local system:
    }else{

    //     - stops the ConnectRetryTimer to zero,
        stateMachine->connectRetryTimer->reset();

    //     TODO drops the TCP connection,

    //     TODO releases all BGP resources, and

    //     - changes its state to Idle.
        stateMachine->changeState(new BGPStateActive(stateMachine));
        break;
    case BGPOpen_with_DelayOpenTimer_running:
        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,
        stateMachine->connectRetryTimer->reset();

        // TODO completes the BGP initialization,

        // - stops and clears the DelayOpenTimer (sets the value to zero),
        stateMachine->delayOpenTimer->reset();

        // TODO sends an OPEN message,

        // TODO sends a KEEPALIVE message,

        // - if the HoldTimer initial value is non-zero,
        if(stateMachine->holdTimer->getRemainingTime() != 0ms){

        //     - starts the KeepaliveTimer with the initial value and
            stateMachine->keepAliveTimer->start();

        //     - resets the HoldTimer to the negotiated value,
            stateMachine->holdTimer->setRemainingTime(0ms); // NOT_SURE

        //   else, if the HoldTimer initial value is zero,
        }else{
        //     - resets the KeepaliveTimer and
            stateMachine->keepAliveTimer->reset();
        //     - resets the HoldTimer value to zero,
            stateMachine->holdTimer->reset();
        }
        // - and changes its state to OpenConfirm.
        stateMachine->changeState(new BGPStateOpenConfirm(stateMachine));
        break;
    case BGPHeaderErr:
    case BGPOpenMsgErr:

        if(stateMachine->getSendNOTIFICATIONwithoutOPEN()){
                //TODO
            // - (optionally) If the SendNOTIFICATIONwithoutOPEN attribute is
            //   set to TRUE, then the local system first sends a NOTIFICATION
            //   message with the appropriate error code, and then
        }

        // - stops the ConnectRetryTimer (if running) and sets the
        //   ConnectRetryTimer to zero,
        stateMachine->connectRetryTimer->reset();

        // TODO releases all BGP resources,

        // TODO drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,
        stateMachine->incrementConnectRetryCounter();

        if(stateMachine->getDampPeerOscillations()){
        // - (optionally) performs peer oscillation damping if the
        //   DampPeerOscillations attribute is set to TRUE, and
        }

        // - changes its state to Idle.
        stateMachine->changeState(new BGPStateIdle(stateMachine));

        break;
    case NotifMsgVerErr:
        if(stateMachine->delayOpenTimer->isRunning()){
            //     - stops the ConnectRetryTimer (if running) and sets the
            //       ConnectRetryTimer to zero,
            stateMachine->connectRetryTimer->reset();

            //     - stops and resets the DelayOpenTimer (sets to zero),
            stateMachine->delayOpenTimer->reset();

            //     TODO releases all BGP resources,

            //     TODO drops the TCP connection, and

            //     - changes its state to Idle.
        

        //   If the DelayOpenTimer is not running, the local system:
        }else{
            //     - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
            //       zero,
            stateMachine->connectRetryTimer->reset();

            //     TODO releases all BGP resources,

            //     TODO drops the TCP connection,

            //     - increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            if(stateMachine->getDampPeerOscillations()){
            //     - performs peer oscillation damping if the DampPeerOscillations
            //       attribute is set to True, and
            }

            //     - changes its state to Idle.
        }
        stateMachine->changeState(new BGPStateIdle(stateMachine));
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
        stateMachine->connectRetryTimer->reset();
        // - if the DelayOpenTimer is running, stops and resets the
        //   DelayOpenTimer (sets to zero),
        stateMachine->delayOpenTimer->reset();

        // TODO releases all BGP resources,

        // TODO drops the TCP connection,

        // - increments the ConnectRetryCounter by 1,
        stateMachine->incrementConnectRetryCounter();

        if(stateMachine->getDampPeerOscillations()){
        // - performs peer oscillation damping if the DampPeerOscillations
        //   attribute is set to True, and
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