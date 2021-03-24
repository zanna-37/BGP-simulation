#include "BGPStateIdle.h"

#include "BGPStateActive.h"
#include "BGPStateConnect.h"
// #include "BGPStateOpenSent.h"
// #include "BGPStateOpenconfirm.h"
// #include "BGPStateEnstablished.h"


BGPStateIdle ::~BGPStateIdle() {}

bool BGPStateIdle ::onEvent(Event event) {
    // maybe curly parenthesis

    bool handled = true;

    switch (event) {
        case ManualStart:
        case AutomaticStart:
            // TODO initializes all BGP resources for the peer connection,

            // - sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // TODO initiates a TCP connection to the other BGP peer,

            // TODO listens for a connection that may be initiated by the remote
            //   BGP peer, and

            // - changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));


            break;
        case ManualStart_with_PassiveTcpEstablishment:
        case AutomaticStart_with_PassiveTcpEstablishment:

            // TODO initializes all BGP resources,

            // - sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // TODO listens for a connection that may be initiated by the remote
            //   peer, and

            // - changes its state to Active.
            stateMachine->changeState(new BGPStateActive(stateMachine));

            break;
        case AutomaticStart_with_DampPeerOscillations:
        case AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
        case IdleHoldTimer_Expires:

            if (stateMachine->getDampPeerOscillations()) {
                // TODO: Do we need to implement DampPeerOscillations?
                // Upon receiving these 3 events, the local system will use
                // these events to prevent peer oscillations.  The method of
                // preventing persistent peer oscillation is outside the scope
                // of this document.
            }

            break;

        default:
            handled = false;
            break;
    }
    return handled;
}