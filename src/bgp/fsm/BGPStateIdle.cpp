#include "BGPStateIdle.h"

#include "../../entities/Router.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "BGPStateActive.h"
#include "BGPStateConnect.h"
#include "BGPStateMachine.h"


bool BGPStateIdle ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event) {
        case BGPEvent::ManualStart:
        case BGPEvent::AutomaticStart:
            // TODO initializes all BGP resources for the peer connection,

            // - sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // TODO initiates a TCP connection to the other BGP peer,
            // initiateTCPConnection();

            // listens for a connection that may be initiated by the remote BGP
            // peer
            stateMachine->connection->listenForRemotelyInitiatedConnections();

            // and changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));

            break;

        case BGPEvent::ManualStart_with_PassiveTcpEstablishment:
        case BGPEvent::AutomaticStart_with_PassiveTcpEstablishment:

            // TODO initializes all BGP resources,

            // - sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // listens for a connection that may be initiated by the remote BGP
            // peer
            stateMachine->connection->listenForRemotelyInitiatedConnections();

            // and changes its state to Active.
            stateMachine->changeState(new BGPStateActive(stateMachine));

            break;

        case BGPEvent::AutomaticStart_with_DampPeerOscillations:
        case BGPEvent::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
        case BGPEvent::IdleHoldTimer_Expires:

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
