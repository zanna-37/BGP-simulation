#include "fsm/BGPStateIdle.h"
#include "fsm/BGPStateConnect.h"
#include "fsm/BGPStateActive.h"
// #include "fsm/BGPStateOpenSent.h"
// #include "fsm/BGPStateOpenconfirm.h"
// #include "fsm/BGPStateEnstablished.h"


BGPStateIdle :: ~BGPStateIdle(){

}

void BGPStateIdle :: enter(){

}
void BGPStateIdle :: execute(){

}

void BGPStateIdle :: exit(){

}

bool BGPStateIdle :: OnEvent(Event event){

    //maybe curly parenthesis

    //Has the event been handled?
    bool handled = false;


    switch (event)
    {
    case ManualStart:
    case AutomaticStart:
        //initializes resources
        //
        stateMachine->setConnectRetryCounter(0);
        //starts ConnectRetryTimer
        //initiates a TCP connection to other BGP peer
        //listens for a connection

        stateMachine->ChangeState(new BGPStateConnect(stateMachine));

        handled = true;
        break;
    case ManualStop:
    case AutomaticStop:
        break;
    case ManualStart_with_PassiveTcpEstablishment:
    case AutomaticStart_with_PassiveTcpEstablishment:

        // - initializes all BGP resources,

        stateMachine->setConnectRetryCounter(0);

        // - starts the ConnectRetryTimer with the initial value,

        // - listens for a connection that may be initiated by the remote
        //   peer, and

        stateMachine->ChangeState(new BGPStateActive(stateMachine));

        handled = true;
        break;
    case AutomaticStart_with_DampPeerOscillations:
    case AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
    case IdleHoldTimer_Expires:

        if(stateMachine->dampPeerOscillations()){
            
            handled = true;
        }

        break;

    default:
        break;
    
    
    }
    return handled;
}