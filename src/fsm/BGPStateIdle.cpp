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

bool BGPStateIdle :: onEvent(Event event){

    //maybe curly parenthesis

    bool handled = true;

    switch (event)
    {
    case ManualStart:
    case AutomaticStart:
        // - initializes all BGP resources for the peer connection,

        // - sets ConnectRetryCounter to zero,

        // - starts the ConnectRetryTimer with the initial value,

        // - initiates a TCP connection to the other BGP peer,

        // - listens for a connection that may be initiated by the remote
        //   BGP peer, and

        // - changes its state to Connect.


        break;
    case ManualStart_with_PassiveTcpEstablishment:
    case AutomaticStart_with_PassiveTcpEstablishment:

        // - initializes all BGP resources,

        // - sets the ConnectRetryCounter to zero,

        // - starts the ConnectRetryTimer with the initial value,

        // - listens for a connection that may be initiated by the remote
        //   peer, and

        // - changes its state to Active.

        break;
    case AutomaticStart_with_DampPeerOscillations:
    case AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
    case IdleHoldTimer_Expires:

        if(stateMachine->getDampPeerOscillations()){
    //   Upon receiving these 3 events, the local system will use these
    //   events to prevent peer oscillations.  The method of preventing
    //   persistent peer oscillation is outside the scope of this document.

        }

        break;

    default:
        handled = false;
        break;
    
    
    }
    return handled;
}