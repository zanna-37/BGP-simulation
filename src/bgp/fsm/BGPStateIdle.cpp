#include "BGPStateIdle.h"

#include "../../entities/Router.h"
#include "../BGPApplication.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "BGPStateActive.h"
#include "BGPStateConnect.h"
#include "BGPStateMachine.h"


bool BGPStateIdle ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventType) {
        case BGPEventType::ManualStart:
        case BGPEventType::AutomaticStart:
            // XXX initializes all BGP resources for the peer connection,  All
            // done?

            // sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // initiates a TCP connection to the other BGP peer,
            stateMachine->connection->asyncConnectToPeer();

            // listens for a connection that may be initiated by the remote BGP
            // peer
            stateMachine->connection->bgpApplication->startListeningOnSocket(
                stateMachine->connection->srcAddr);

            // and changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));
            break;

        case BGPEventType::ManualStop:
        case BGPEventType::AutomaticStop:
            // The events must be ignored in Idle state
            break;

        case BGPEventType::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventType::AutomaticStart_with_PassiveTcpEstablishment:
            // XXX initializes all BGP resources, All done?

            // sets the ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // starts the ConnectRetryTimer with the initial value,
            stateMachine->connectRetryTimer->start();

            // listens for a connection that may be initiated by the remote BGP
            // peer
            stateMachine->connection->bgpApplication->startListeningOnSocket(
                stateMachine->connection->srcAddr);

            // and changes its state to Active.
            stateMachine->changeState(new BGPStateActive(stateMachine));
            break;

        case BGPEventType::AutomaticStart_with_DampPeerOscillations:
        case BGPEventType::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
        case BGPEventType::IdleHoldTimer_Expires:
            if (stateMachine->getDampPeerOscillations()) {
                // XXX: Do we need to implement DampPeerOscillations?
                // Upon receiving these 3 events, the local system will use
                // these events to prevent peer oscillations.  The method of
                // preventing persistent peer oscillation is outside the scope
                // of this document.
            }

            handled = false;
            break;
        case BGPEventType::ConnectRetryTimer_Expires:
        case BGPEventType::HoldTimer_Expires:
        case BGPEventType::KeepaliveTimer_Expires:
        case BGPEventType::DelayOpenTimer_Expires:
        case BGPEventType::Tcp_CR_Invalid:
        case BGPEventType::Tcp_CR_Acked:
        case BGPEventType::TcpConnectionConfirmed:
        case BGPEventType::TcpConnectionFails:
        case BGPEventType::BGPOpen:
        case BGPEventType::BGPOpen_with_DelayOpenTimer_running:
        case BGPEventType::BGPHeaderErr:
        case BGPEventType::BGPOpenMsgErr:
        case BGPEventType::OpenCollisionDump:
        case BGPEventType::NotifMsgVerErr:
        case BGPEventType::NotifMsg:
        case BGPEventType::KeepAliveMsg:
        case BGPEventType::UpdateMsg:
        case BGPEventType::UpdateMsgErr:
            // (Events 9-12, 15-28) received in the Idle state
            // does not cause change in the state of the local system.
            break;

        case BGPEventType::SendUpdateMsg:
            // Event for checking that the fsm is in a good state before sending
            // the message
            L_ERROR(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                "UPDATE message cannot be sent in Idle state");
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
