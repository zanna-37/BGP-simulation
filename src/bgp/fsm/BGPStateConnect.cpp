#include "BGPStateConnect.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <stack>
#include <utility>

#include "../../entities/Router.h"
#include "../../utils/Timer.h"
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

    switch (event) {
        case BGPEvent::ManualStop:
            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // TODO releases all BGP resources,

            // sets ConnectRetryCounter to zero,
            stateMachine->setConnectRetryCounter(0);

            // stops the ConnectRetryTimer and sets ConnectRetryTimer to zero
            stateMachine->resetConnectRetryTimer();

            // and changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEvent::ConnectRetryTimer_Expires:
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

        case BGPEvent::DelayOpenTimer_Expires:
            // TODO sends an OPEN message to its peer,

            // sets the HoldTimer to a large value, and
            stateMachine->setHoldTime(
                BGPStateMachine::kHoldTime_large_defaultVal);
            stateMachine->holdTimer->start();  // TODO do we need to start it?

            // changes its state to OpenSent.
            stateMachine->changeState(new BGPStateOpenSent(stateMachine));

            handled = false;
            break;

        case BGPEvent::TcpConnection_Valid:
            // TODO the TCP connection is processed, OPTIONAL
            break;

        case BGPEvent::Tcp_CR_Invalid:
            // TODO the local system rejects the TCP connection, OPTIONAL
            break;

        case BGPEvent::Tcp_CR_Acked:
        case BGPEvent::TcpConnectionConfirmed: {
            if (stateMachine->getDelayOpen()) {
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

                // TODO completes BGP initialization

                // TODO sends an OPEN message to its peer,

                // FIXME correct the hardcoded AS_number
                std::unique_ptr<BGPLayer> bgpOpenLayer =
                    std::make_unique<BGPOpenLayer>(
                        /* TODO wait for bgp-table branch to be merged then put
                           stateMachine->connection->owner->AS_number*/
                        1111,
                        (uint16_t)(stateMachine->getHoldTime().count()),
                        pcpp::IPv4Address(stateMachine->connection->srcAddr));
                bgpOpenLayer->computeCalculateFields();

                std::unique_ptr<std::stack<std::unique_ptr<pcpp::Layer>>>
                    layers =
                        make_unique<std::stack<std::unique_ptr<pcpp::Layer>>>();
                layers->push(std::move(bgpOpenLayer));

                stateMachine->connection->sendData(std::move(layers));

                // sets the HoldTimer to a large value, and
                stateMachine->setHoldTime(
                    BGPStateMachine::kHoldTime_large_defaultVal);
                stateMachine->holdTimer
                    ->start();  // TODO do we need to start it?

                // changes its state to OpenSent.
                stateMachine->changeState(new BGPStateOpenSent(stateMachine));

                handled = false;
            }
            break;
        }
        case BGPEvent::TcpConnectionFails:
            // If the DelayOpenTimer is running, the local system:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
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

                // TODO releases all BGP resources, and

                // changes its state to Idle.
                stateMachine->changeState(new BGPStateActive(stateMachine));
            }
            break;

        case BGPEvent::BGPOpen_with_DelayOpenTimer_running:
            // stops the ConnectRetryTimer (if running) and sets the
            // ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO completes the BGP initialization,

            // stops and clears the DelayOpenTimer (sets the value to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO sends an OPEN message,

            // TODO sends a KEEPALIVE message,

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
            stateMachine->changeState(new BGPStateOpenConfirm(stateMachine));

            handled = false;
            break;

        case BGPEvent::BGPHeaderErr:
        case BGPEvent::BGPOpenMsgErr:
            if (stateMachine->getSendNOTIFICATIONwithoutOPEN()) {
                // TODO
                // - (optionally) If the SendNOTIFICATIONwithoutOPEN attribute
                // is
                //   set to TRUE, then the local system first sends a
                //   NOTIFICATION message with the appropriate error code, and
                //   then
            }

            // stops the ConnectRetryTimer (if running) and sets the
            // ConnectRetryTimer to zero,
            stateMachine->resetConnectRetryTimer();

            // TODO releases all BGP resources,

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
                // performs peer oscillation damping
                // <-- Not implemented
            }

            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));

            handled = false;
            break;

        case BGPEvent::NotifMsgVerErr:
            if (stateMachine->delayOpenTimer->getState() == TICKING) {
                // stops the ConnectRetryTimer (if running) and sets the
                // ConnectRetryTimer to zero,
                stateMachine->resetConnectRetryTimer();

                // stops and resets the DelayOpenTimer (sets to zero),
                stateMachine->resetDelayOpenTimer();

                // TODO releases all BGP resources,

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);
            } else {
                // If the DelayOpenTimer is not running, the local system:
                // stops the ConnectRetryTimer and sets the ConnectRetryTimer to
                // zero,
                stateMachine->resetConnectRetryTimer();

                // TODO releases all BGP resources,

                // drops the TCP connection,
                stateMachine->connection->dropConnection(false);

                // increments the ConnectRetryCounter by 1,
                stateMachine->incrementConnectRetryCounter();

                // optionally performs peer oscillation damping if the
                // DampPeerOscillations attribute is set to TRUE, and
                if (stateMachine->getDampPeerOscillations()) {
                    // performs peer oscillation damping
                    // <-- Not implemented
                }
            }
            // changes its state to Idle.
            stateMachine->changeState(new BGPStateIdle(stateMachine));
            break;

        case BGPEvent::AutomaticStop:
        case BGPEvent::HoldTimer_Expires:
        case BGPEvent::KeepaliveTimer_Expires:
        case BGPEvent::BGPOpen:
        case BGPEvent::OpenCollisionDump:
        case BGPEvent::NotifMsg:
        case BGPEvent::KeepAliveMsg:
        case BGPEvent::UpdateMsg:
        case BGPEvent::UpdateMsgErr:
            // if the ConnectRetryTimer is running, stops and resets the
            // ConnectRetryTimer (sets to zero),
            stateMachine->resetConnectRetryTimer();
            // if the DelayOpenTimer is running, stops and resets the
            // DelayOpenTimer (sets to zero),
            stateMachine->resetDelayOpenTimer();

            // TODO releases all BGP resources,

            // drops the TCP connection,
            stateMachine->connection->dropConnection(false);

            // increments the ConnectRetryCounter by 1,
            stateMachine->incrementConnectRetryCounter();

            // optionally performs peer oscillation damping if the
            // DampPeerOscillations attribute is set to TRUE, and
            if (stateMachine->getDampPeerOscillations()) {
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
