#include "BGPStateIdle.h"
#include "BGPStateConnect.h"
#include "BGPStateActive.h"
// #include "BGPStateOpenSent.h"
// #include "BGPStateOpenconfirm.h"
// #include "BGPStateEnstablished.h"


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
    bool handled = true;


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

        break;
    case ManualStart_with_PassiveTcpEstablishment:
    case AutomaticStart_with_PassiveTcpEstablishment:

        // - initializes all BGP resources,

        stateMachine->setConnectRetryCounter(0);

        // - starts the ConnectRetryTimer with the initial value,

        // - listens for a connection that may be initiated by the remote
        //   peer, and

        stateMachine->ChangeState(new BGPStateActive(stateMachine));

        break;
    case AutomaticStart_with_DampPeerOscillations:
    case AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
    case IdleHoldTimer_Expires:

        if(stateMachine->dampPeerOscillations()){
            
        }

        break;

    default:
        handled = false;
        break;
    
    
    }
    return handled;
}