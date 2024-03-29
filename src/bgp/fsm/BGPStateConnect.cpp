#include "BGPStateConnect.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <stack>
#include <utility>

#include "../../entities/Router.h"
#include "../../utils/Timer.h"
#include "../BGPApplication.h"
#include "../BGPConnection.h"
#include "../BGPEvent.h"
#include "../BGPTimer.h"
#include "../packets/BGPLayer.h"
#include "../packets/BGPOpenLayer.h"
#include "BGPStateActive.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"
#include "BGPStateOpenConfirm.h"
#include "BGPStateOpenSent.h"
#include "IpAddress.h"
#include "Layer.h"


bool BGPStateConnect ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventType) {
        case BGPEventType::ManualStart:
        case BGPEventType::AutomaticStart:
        case BGPEventType::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventType::AutomaticStart_with_PassiveTcpEstablishment:
        case BGPEventType::AutomaticStart_with_DampPeerOscillations:
        case BGPEventType::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            // (Events 1, 3-7) are ignored in the Connect state.
            break;
        case BGPEventType::ManualStop:
            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // XXX releases all BGP resources, It should be done.
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // stops the ConnectRetryTimer and sets ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // and changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::ConnectRetryTimer_Expires:
            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);
            // restarts the ConnectRetryTimer,
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // stops the DelayOpenTimer and resets the timer to zero,
            stateMachine->resetDelayOpenTimer();

            // initiates a TCP connection to the other BGP peer,
            stateMachine->connection->asyncConnectToPeer();

            // continues to listen for a connection that may be initiated by the
            // remote BGP peer
            // --> Nothing to do as the listening is already taking place.
            break;

        case BGPEventType::DelayOpenTimer_Expires:
            // Optional events
            // XXX sends an OPEN message to its peer,

            // sets the HoldTimer to a large value, and
            stateMachine->setNegotiatedHoldTime(
                BGPStateMachine::kHoldTime_large_defaultVal);
            stateMachine->holdTimer->start();  // XXX do we need to start it?

            // changes its state to OpenSent.
            stateMachine->changeState(new BGPStateOpenSent(stateMachine));

            handled = false;
            break;

        case BGPEventType::TcpConnection_Valid:
            // XXX the TCP connection is processed, OPTIONAL
            handled = false;
            break;

        case BGPEventType::Tcp_CR_Invalid:
            // XXX the local system rejects the TCP connection, OPTIONAL
            handled = false;
            break;

        case BGPEventType::Tcp_CR_Acked:
        case BGPEventType::TcpConnectionConfirmed:
            if (stateMachine->getDelayOpen()) {
                // OPTIONAL
                // If the DelayOpen attribute is set to TRUE, the local system:
                // stops the ConnectRetryTimer (if running) and sets the
                // ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // sets the DelayOpenTimer to the initial value, and
                stateMachine->delayOpenTimer->start();

                // stays in the Connect state.
            } else {
                // If the DelayOpen attribute is set to FALSE, the local system:
                // stops the ConnectRetryTimer (if running) and sets the
                // ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // XXX completes BGP initialization, should be done.

                // TODO the hold time in the open message is the same of the one
                // setted as a large time? or it should be the old one? sends an

                // OPEN message to its peer,
                std::unique_ptr<BGPLayer> bgpOpenLayer =
                    std::make_unique<BGPOpenLayer>(
                        stateMachine->connection->owner->AS_number,
                        stateMachine->getNegotiatedHoldTime().count(),
                        stateMachine->connection->bgpApplication
                            ->getBGPIdentifier());
                bgpOpenLayer->computeCalculateFields();

                // Commented code below is for tests
                /* BGPOpenLayer bgpOpenMessage =
                    BGPOpenLayer(stateMachine->connection->owner->AS_number,
                                 stateMachine->getHoldTime().count(),
                                 stateMachine->connection->bgpApplication
                                     ->getBGPIdentifier());
                std::cout << bgpOpenMessage.toString() << std::endl; */

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpOpenLayer));

                stateMachine->connection->sendData(std::move(layers));
                L_INFO_CONN(stateMachine->connection->owner->ID + " " +
                                stateMachine->name,
                            stateMachine->connection->toString(),
                            "Sending OPEN message");
                // sets the HoldTimer to a large value, and
                stateMachine->setNegotiatedHoldTime(
                    BGPStateMachine::kHoldTime_large_defaultVal);
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer
                    ->start();  // TODO do we need to start it?

                // changes its state to OpenSent.
                stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            }
            break;

        case BGPEventType::TcpConnectionFails:
            // If the DelayOpenTimer is running, the local system:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                // OPTIONAL
                // restarts the ConnectRetryTimer with the initial value,
                stateMachine->resetConnectRetryTimer();
                stateMachine->connectRetryTimer->start();

                // stops the DelayOpenTimer and resets its value to zero,
                stateMachine->resetDelayOpenTimer();

                // continues to listen for a connection that may be initiated by
                // the remote BGP peer, and
                // --> Nothing to do as the listening is already taking place.

                // changes its state to Active.
                stateMachine->changeState(new BGPStateActive(stateMachine));
            } else {
                // If the DelayOpenTimer is not running, the local system:
                // stops the ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);

                // XXX releases all BGP resources, and - should be done
                stateMachine->connection->bgpApplication->stopListeningOnSocket(
                    stateMachine->connection->srcAddr);

                // changes its state to Idle.
                stateMachine->changeState(new BGPStateIdle(stateMachine));
            }
            break;

        case BGPEventType::BGPOpen_with_DelayOpenTimer_running:
            // OPTIONAL
            /* // stops the ConnectRetryTimer (if running) and sets the
            // ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX completes the BGP initialization,

            // stops and clears the DelayOpenTimer (sets the value to zero),
            stateMachine->resetDelayOpenTimer();

            // XXX sends an OPEN message,

            // XXX sends a KEEPALIVE message,

            if (stateMachine->holdTimer->getDuration() != 0ms) {
                // if the HoldTimer initial value is non-zero, // FIXME
                // starts the KeepaliveTimer with the initial value and
                stateMachine->resetKeepAliveTimer();  // ← probably not needed
                stateMachine->keepAliveTimer->start();

                // resets the HoldTimer to the negotiated value,
                // FIXME now it is reset to the default value, not the
                // negotiated one
                stateMachine->resetHoldTimer();
                // stateMachine->holdTimer->setRemainingTime(xxx); // NOT_SURE
                stateMachine->holdTimer->start();
            } else {
                // else, if the HoldTimer initial value is zero,
                // resets the KeepaliveTimer and
                stateMachine->resetKeepAliveTimer();
                // resets the HoldTimer value to zero,
                stateMachine->resetHoldTimer();
            }
            // - and changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenConfirm(stateMachine)); */

            handled = false;
            break;

        case BGPEventType::BGPHeaderErr:
        case BGPEventType::BGPOpenMsgErr:
            if (stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // OPTIONAL
                // - (optionally) If the SendNOTIFICATIONwithoutOPEN attribute
                // is
                //   set to TRUE, then the local system first sends a
                //   NOTIFICATION message with the appropriate error code, and
                //   then
            }

            // stops the ConnectRetryTimer (if running) and sets the
            // ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // XXX releases all BGP resources, - done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::NotifMsgVerErr:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                // OPTIONAL
                // stops the ConnectRetryTimer (if running) and sets the
                // ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // stops and resets the DelayOpenTimer (sets to zero),
                stateMachine->resetDelayOpenTimer();

                // XXX releases all BGP resources,
                stateMachine->connection->bgpApplication->stopListeningOnSocket(
                    stateMachine->connection->srcAddr);

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);
                // changes its state to Idle.
                stateMachine->changeState(new BGPStateIdle(stateMachine));
            } else {
                // If the DelayOpenTimer is not running, the local system:
                // stops the ConnectRetryTimer and sets the ConnectRetryTimer to
                // zero,
                stateMachine->resetConnectRetryTimer();

                // XXX releases all BGP resources, done
                stateMachine->connection->bgpApplication->stopListeningOnSocket(
                    stateMachine->connection->srcAddr);

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);

                // increments the ConnectRetryCounter by 1,
                stateMachine->incrementConnectRetryCounter();

                // optionally performs peer oscillation damping if the
                // DampPeerOscillations attribute is set to TRUE, and
                if (stateMachine->getDampPeerOscillations()) {
                    // OPTIONAL
                    // performs peer oscillation damping
                    // <-- Not implemented
                }
                // changes its state to Idle.
                stateMachine->changeState(new BGPStateIdle(stateMachine));
            }
            break;

        case BGPEventType::AutomaticStop:
        case BGPEventType::HoldTimer_Expires:
        case BGPEventType::KeepaliveTimer_Expires:
        case BGPEventType::IdleHoldTimer_Expires:
        case BGPEventType::BGPOpen:
        case BGPEventType::OpenCollisionDump:
        case BGPEventType::NotifMsg:
        case BGPEventType::KeepAliveMsg:
        case BGPEventType::UpdateMsg:
        case BGPEventType::UpdateMsgErr:
            // if the ConnectRetryTimer is running, stops and resets the
            // ConnectRetryTimer (sets to zero),
            stateMachine->resetConnectRetryTimer();
            // if the DelayOpenTimer is running, stops and resets the
            // DelayOpenTimer (sets to zero),
            stateMachine
                ->resetDelayOpenTimer();  // This is an optional attribute

            // XXX releases all BGP resources, done
            stateMachine->connection->bgpApplication->stopListeningOnSocket(
                stateMachine->connection->srcAddr);

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventType::SendUpdateMsg:
            // Event for checking that the fsm is in a good state before sending
            // the message
            L_ERROR_CONN(
                stateMachine->connection->owner->ID + " " + stateMachine->name,
                stateMachine->connection->toString(),
                "UPDATE message cannot be sent in Connect state");
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
