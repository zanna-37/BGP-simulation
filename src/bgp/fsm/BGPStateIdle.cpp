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
            // TODO initializes all BGP resources for the peer connection,  All
            // done?

            // sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // initiates a TCP connection to the other BGP peer,
            stateMachine->connection->asyncConnectToPeer();

            // listens for a connection that may be initiated by the remote BGP
            // peer
            stateMachine->connection->listenForRemotelyInitiatedConnections();

            // and changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));
            break;

        case BGPEvent::ManualStop:
        case BGPEvent::AutomaticStop:
            // The events must be ingnored in Indle state
            break;

        case BGPEvent::ManualStart_with_PassiveTcpEstablishment:
        case BGPEvent::AutomaticStart_with_PassiveTcpEstablishment:

            // TODO initializes all BGP resources, All done?

            // sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // starts the ConnectRetryTimer with the initial value,
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

            handled = false;
            break;
        case BGPEvent::ConnectRetryTimer_Expires:
        case BGPEvent::HoldTimer_Expires:
        case BGPEvent::KeepaliveTimer_Expires:
        case BGPEvent::DelayOpenTimer_Expires:
        case BGPEvent::Tcp_CR_Invalid:
        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed:
        case BGPEvent::TcpConnectionFails:
        case BGPEvent::BGPOpen:
        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:
        case BGPEvent::OpenCollisionDump:
        case BGPEvent::NotifMsgVerErr:
        case BGPEvent::NotifMsg:
        case BGPEvent::KeepAliveMsg:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
            // (Events 9-12, 15-28) received in the Idle state
            // does not cause change in the state of the local system.
            break;
        default:
            handled = false;
            break;
    }
    return handled;
}
