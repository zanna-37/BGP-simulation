#include "BGPStateActive.h"

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
#include "BGPStateConnect.h"
#include "BGPStateIdle.h"
#include "BGPStateMachine.h"
#include "BGPStateOpenSent.h"
#include "IpAddress.h"
#include "Layer.h"


bool BGPStateActive ::onEvent(BGPEvent event) {
    bool handled = true;

    switch (event.eventList) {
        case BGPEventList::ManualStart:
        case BGPEventList::AutomaticStart:
        case BGPEventList::ManualStart_with_PassiveTcpEstablishment:
        case BGPEventList::AutomaticStart_with_PassiveTcpEstablishment:
        case BGPEventList::AutomaticStart_with_DampPeerOscillations:
        case BGPEventList::
            AutomaticStart_with_DampPeerOscillations_and_PassiveTcpEstablishment:
            // (Events 1, 3-7) are ignored in the Active state.
            break;
        case BGPEventList::ManualStop:
            if (stateMachine->delayOpenTimer->getState() == TICKING &&
                stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // - If the DelayOpenTimer is running and the
                //   SendNOTIFICATIONwithoutOPEN session attribute is set, the
                //   local system sends a NOTIFICATION with a Cease,
            }

            // TODO releases all BGP resources including stopping the
            //   DelayOpenTimer
            stateMachine->resetDelayOpenTimer();  // optional value

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // - sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // - stops the ConnectRetryTimer and sets the ConnectRetryTimer to
            //   zero, and
            stateMachine->resetConnectRetryTimer();

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::ConnectRetryTimer_Expires:
            //  - restarts the ConnectRetryTimer (with initial value),
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // initiates a TCP connection to the other BGP peer,
            stateMachine->connection->asyncConnectToPeer();

            // Continues to listen for a TCP connection that may be initiated by
            // a remote BGP peer.
            // --> Nothing to do as the listening is already taking place.

            // and changes its state to Connect.
            stateMachine->changeState(new BGPStateConnect(stateMachine));
            break;

        case BGPEventList::DelayOpenTimer_Expires:
            // OPTIONAL
            /* //  - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // - stops and clears the DelayOpenTimer (set to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO completes the BGP initialization,

            // TODO sends the OPEN message to its remote peer,

            // sets its hold timer to a large value ( A HoldTimer value of 4
            // minutes is also suggested for this state transition.), and
            stateMachine->holdTimer =
                new BGPTimer("holdTimer",
                             stateMachine,
                             BGPEventList::HoldTimer_Expires,
                             BGPStateMachine::kHoldTime_large_defaultVal);
            stateMachine->holdTimer->start();

            // - changes its state to OpenSent
            stateMachine->changeState(new BGPStateOpenSent(stateMachine)); */
            handled = false;
            break;

        case BGPEventList::TcpConnection_Valid:
            // OPTIONAL
            // TODO the local system processes the TCP connection flags and
            // stays in the Active state.
            handled = false;
            break;

        case BGPEventList::Tcp_CR_Invalid:
            // OPTIONAL
            // TODO the local system rejects the TCP connection and stays in the
            // Active State.
            handled = false;
            break;

        case BGPEventList::Tcp_CR_Acked:
        case BGPEventList::TcpConnectionConfirmed: {
            // If the DelayOpen attribute is set to TRUE, the local system:
            if (stateMachine->getDelayOpen()) {
                // OPTIONAL
                // stops the ConnectRetryTimer and sets the ConnectRetryTimer to
                // zero
                stateMachine->resetConnectRetryTimer();

                // Sets the DelayOpenTimer to the initial value (DelayOpenTime),
                // and
                stateMachine->resetDelayOpenTimer();
                stateMachine->delayOpenTimer->start();

                // stays in the Active state.

            } else {
                // If the DelayOpen attribute is set to FALSE, the local system:

                // ets the ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                //   TODO completes the BGP initialization, should be done

                //   sends the OPEN message to its peer,1
                std::unique_ptr<BGPLayer> bgpOpenLayer =
                    std::make_unique<BGPOpenLayer>(
                        stateMachine->connection->owner->AS_number,
                        stateMachine->getHoldTime().count(),
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

                // sets its HoldTimer to a large value, and
                stateMachine->resetHoldTimer();
                stateMachine->holdTimer->setDuration(
                    BGPStateMachine::kHoldTime_large_defaultVal);
                stateMachine->holdTimer
                    ->start();  // TODO do we need to start the timer?

                // changes its state to OpenSent.
                stateMachine->changeState(new BGPStateOpenSent(stateMachine));
            }
            break;
        }
        case BGPEventList::TcpConnectionFails:
            // restarts the ConnectRetryTimer (with the initial value),
            stateMachine->resetConnectRetryTimer();
            stateMachine->connectRetryTimer->start();

            // stops and clears the DelayOpenTimer (sets the value to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO releases all BGP resource, - done

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

        case BGPEventList::BGPOpen_with_DelayOpenTimer_running:
            // OPTIONAL
            /* // stops the ConnectRetryTimer (if running) and sets the
            // ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // stops and clears the DelayOpenTimer (sets to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO completes the BGP initialization,

            // TODO sends an OPEN message,

            // TODO sends a KEEPALIVE message,

            // - if the HoldTimer value is non-zero, // FIXME
            if (stateMachine->holdTimer->getDuration() != 0ms) {
                // starts the KeepaliveTimer to initial value,
                stateMachine->resetKeepAliveTimer();  // ← probably not needed
                stateMachine->keepAliveTimer->start();

                // resets the HoldTimer to the negotiated value,
                // FIXME now it is reset to the default value, not the
                // negotiated one
                stateMachine->resetHoldTimer();
                // stateMachine->holdTimer->setRemainingTime(xxx); // NOT_SURE
                stateMachine->holdTimer->start();
            } else {
                // else if the HoldTimer is zero
                // resets the KeepaliveTimer (set to zero),
                stateMachine->resetKeepAliveTimer();

                // resets the HoldTimer to zero, and
                stateMachine->resetHoldTimer();
            }

            // - changes its state to OpenConfirm.
            stateMachine->changeState(new BGPStateOpenSent(stateMachine)); */

            handled = false;
            break;

        case BGPEventList::BGPHeaderErr:
        case BGPEventList::BGPOpenMsgErr:
            //  - (optionally) sends a NOTIFICATION message with the appropriate
            //   error code if the SendNOTIFICATIONwithoutOPEN attribute is set
            //   to TRUE,
            if (stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // OPTIONAL
                // TODO
            }

            // - sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // - changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEventList::NotifMsgVerErr:

            /*    the local system checks the DelayOpenTimer.  If the
               DelayOpenTimer is running, the local system:

                - stops the ConnectRetryTimer (if running) and sets the
                ConnectRetryTimer to zero,

                - stops and resets the DelayOpenTimer (sets to zero),

                - releases all BGP resources,

                - drops the TCP connection, and

                - changes its state to Idle. */

            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                // OPTIONAL
            } else {
                // stops the ConnectRetryTimer (if running) and sets the
                // ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // TODO releases all BGP resources, done

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);

                // increments the ConnectRetryCounter by 1,
                stateMachine->incrementConnectRetryCounter();

                // (optionally) performs peer oscillation damping if the
                // DampPeerOscillations attribute is set to TRUE, and
                if (stateMachine->getDampPeerOscillations()) {
                    // OPTIONAL
                }

                // changes its state to Idle.
                stateMachine->changeState(new BGPStateIdle(stateMachine));
            }
            break;

        case BGPEventList::AutomaticStop:
        case BGPEventList::HoldTimer_Expires:
        case BGPEventList::KeepaliveTimer_Expires:
        case BGPEventList::IdleHoldTimer_Expires:
        case BGPEventList::BGPOpen:
        case BGPEventList::OpenCollisionDump:
        case BGPEventList::NotifMsg:
        case BGPEventList::KeepAliveMsg:
        case BGPEventList::UpdateMsg:
        case BGPEventList::UpdateMsgErr:

            // sets the ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources, done

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by one,
            stateMachine->incrementConnectRetryCounter();

            // - (optionally) performs peer oscillation damping if the
            //   DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // OPTIONAL
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        default:
            handled = false;
            break;
    }
    return handled;
}
